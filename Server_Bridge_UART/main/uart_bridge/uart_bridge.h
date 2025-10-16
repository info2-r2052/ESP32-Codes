#pragma once

#include <stdint.h>

#define UART_NUM UART_NUM_1
#define BUF_SIZE 1024
#define TIMEOUT_UART_MS 1000
#define PIN_UART_FAIL GPIO_NUM_2

void init_uart();
void init_fail_gpio();
void send_to_uart_and_respond(int sock, const uint8_t *data, int len);
