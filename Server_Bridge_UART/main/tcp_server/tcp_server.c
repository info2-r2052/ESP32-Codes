// tcp_server.c
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "tcp_server";

#define TCP_PORT 10234
#define BUF_SIZE 256


int client_sock = -1; // Socket cliente actual

static void uart_to_tcp_task(void *arg) {
    uint8_t buf[BUF_SIZE];
    while (1) {
        if (client_sock < 0) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        int len = uart_read_bytes(UART_NUM_1, buf, BUF_SIZE, pdMS_TO_TICKS(100));
        if (len > 0) {
            int sent = send(client_sock, buf, len, 0);
            if (sent < 0) {
                ESP_LOGE(TAG, "Error enviando datos al cliente TCP, cerrando socket");
                shutdown(client_sock, 0);
                close(client_sock);
                client_sock = -1;
            }
        }
    }
}

static void tcp_to_uart_task(void *arg) {
    uint8_t buf[BUF_SIZE];
    while (1) {
        if (client_sock < 0) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        int len = recv(client_sock, buf, BUF_SIZE, 0);
        if (len > 0) {
            uart_write_bytes(UART_NUM_1, (const char *)buf, len);
        } else if (len == 0) {
            ESP_LOGI(TAG, "Cliente TCP desconectado");
            shutdown(client_sock, 0);
            close(client_sock);
            client_sock = -1;
        } else {
            ESP_LOGE(TAG, "Error en recv(), cerrando socket");
            shutdown(client_sock, 0);
            close(client_sock);
            client_sock = -1;
        }
    }
}

void start_tcp_server(void) {
    ESP_LOGI(TAG, "Iniciando servidor TCP en puerto %d", TCP_PORT);

    int listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Error creando socket");
        return;
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(TCP_PORT),
    };

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG, "Error en bind");
        close(listen_sock);
        return;
    }

    if (listen(listen_sock, 1) != 0) {
        ESP_LOGE(TAG, "Error en listen");
        close(listen_sock);
        return;
    }

    ESP_LOGI(TAG, "Servidor TCP escuchando...");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0) {
            ESP_LOGE(TAG, "Error en accept");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        ESP_LOGI(TAG, "Cliente TCP conectado");

        xTaskCreate(uart_to_tcp_task, "uart_to_tcp_task", 4096, NULL, 10, NULL);
        xTaskCreate(tcp_to_uart_task, "tcp_to_uart_task", 4096, NULL, 10, NULL);

        while (client_sock >= 0) {
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        ESP_LOGI(TAG, "Cliente desconectado, esperando nuevo cliente");
    }
}
