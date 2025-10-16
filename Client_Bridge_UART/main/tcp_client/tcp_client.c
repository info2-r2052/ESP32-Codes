// tcp_client/tcp_client.c
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"

#include "tcp_client.h"

static const char *TAG = "tcp_client";

#define SERVER_IP   "192.168.1.162"   // 👈 Cambiá esto a la IP del servidor destino
#define SERVER_PORT 10234
#define BUF_SIZE 256

static int sock = -1;

static void uart_to_tcp_task(void *arg) {
    uint8_t buf[BUF_SIZE];
    while (1) {
        if (sock < 0) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        int len = uart_read_bytes(UART_NUM_1, buf, BUF_SIZE, pdMS_TO_TICKS(100));
        if (len > 0) {
            int sent = send(sock, buf, len, 0);
            if (sent < 0) {
                ESP_LOGE(TAG, "Error enviando datos al servidor");
                close(sock);
                sock = -1;
            }
        }
    }
}

static void tcp_to_uart_task(void *arg) {
    uint8_t buf[BUF_SIZE];
    while (1) {
        if (sock < 0) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        int len = recv(sock, buf, BUF_SIZE, 0);
        if (len > 0) {
            uart_write_bytes(UART_NUM_1, (const char *)buf, len);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Servidor cerró la conexión");
            close(sock);
            sock = -1;
        } else {
            ESP_LOGE(TAG, "Error en recv()");
            close(sock);
            sock = -1;
        }
    }
}

void start_tcp_client(void) {
    while (1) {
        ESP_LOGI(TAG, "Intentando conectar a %s:%d", SERVER_IP, SERVER_PORT);

        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Error creando socket");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(SERVER_PORT),
        };
        inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

        int err = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (err != 0) {
            ESP_LOGE(TAG, "Error al conectar: errno %d", errno);
            close(sock);
            sock = -1;
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        ESP_LOGI(TAG, "Conectado al servidor!");

        // Lanzamos las tareas de comunicación
        xTaskCreate(uart_to_tcp_task, "uart_to_tcp_task", 4096, NULL, 10, NULL);
        xTaskCreate(tcp_to_uart_task, "tcp_to_uart_task", 4096, NULL, 10, NULL);

        // Esperar hasta que se pierda conexión
        while (sock >= 0) {
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        ESP_LOGI(TAG, "Reconectando...");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
