// main/main.c

#include "nvs_flash.h"
#include "wifi.h"
#include "uart_bridge.h"
#include "tcp_server.h"

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    init_wifi();
    init_uart();
    init_fail_gpio();
    start_tcp_server();
}
