#include "wifi.h"
#include <string.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <nvs.h>

#define DEFAULT_SSID "Cletosa&Emmosa"
#define DEFAULT_PASS "Jul14nch0M3rl1n4&Ch0l4"

//#define DEFAULT_SSID "Info2"
//#define DEFAULT_PASS "info2r2052"


static const char *TAG = "wifi";
static esp_netif_t *sta_netif = NULL;

static void save_wifi_credentials(const char *ssid, const char *pass) {
    nvs_handle_t nvs;
    if (nvs_open("storage", NVS_READWRITE, &nvs) == ESP_OK) {
        nvs_set_str(nvs, "ssid", ssid);
        nvs_set_str(nvs, "pass", pass);
        nvs_commit(nvs);
        nvs_close(nvs);
    }
}

static void load_wifi_credentials(char *ssid, char *pass) {
    nvs_handle_t nvs;
    size_t ssid_len = 32, pass_len = 64;
    if (nvs_open("storage", NVS_READONLY, &nvs) == ESP_OK) {
        if (nvs_get_str(nvs, "ssid", ssid, &ssid_len) != ESP_OK)
            strcpy(ssid, DEFAULT_SSID);
        if (nvs_get_str(nvs, "pass", pass, &pass_len) != ESP_OK)
            strcpy(pass, DEFAULT_PASS);
        nvs_close(nvs);
    } else {
        strcpy(ssid, DEFAULT_SSID);
        strcpy(pass, DEFAULT_PASS);
    }
}

static void on_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    const esp_netif_ip_info_t* ip_info = &event->ip_info;
    ESP_LOGI("NETWORK", "IP: " IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI("NETWORK", "Gateway: " IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI("NETWORK", "Netmask: " IPSTR, IP2STR(&ip_info->netmask));
}

static void on_wifi_disconnect(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_LOGW(TAG, "WiFi desconectado. Reintentando...");
    esp_wifi_connect();
}

void init_wifi() {
    esp_netif_init();
    esp_event_loop_create_default();
    sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);

    char ssid[33], pass[65];
    load_wifi_credentials(ssid, pass);

    wifi_config_t wifi_config = { 0 };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL);
    esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL);

    esp_wifi_start();
    esp_wifi_connect();
    ESP_LOGI(TAG, "WiFi inicializado con SSID: %s", ssid);
}

void wifi_reset_credentials() {
    nvs_handle_t nvs;
    if (nvs_open("storage", NVS_READWRITE, &nvs) == ESP_OK) {
        nvs_erase_key(nvs, "ssid");
        nvs_erase_key(nvs, "pass");
        nvs_commit(nvs);
        nvs_close(nvs);
        ESP_LOGI(TAG, "Credenciales WiFi eliminadas de NVS");
    }
}