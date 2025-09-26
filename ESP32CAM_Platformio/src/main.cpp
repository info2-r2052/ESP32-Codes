#include <WiFi.h>
#include <esp_camera.h>
#include <WebServer.h>

// Parámetros de la red WiFi
//const char* ssid = "Cletosa&Emmosa";
//const char* password = "Jul14nch0M3rl1n4&Ch0l4";
const char* ssid = "The Charlyfon";
const char* password = "buffet123";


// Pines de la cámara (ajustar según el modelo)
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

WebServer server(80);

void StartCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;  // 20 MHz para un rendimiento óptimo
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
    Serial.println("Camera init succeeded");
}

void handle_jpg_stream() {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    server.sendContent(response);

    while (server.client().connected()) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            server.send(503, "text/plain", "Camera capture failed");
            return;
        }

        response = "--frame\r\n";
        response += "Content-Type: image/jpeg\r\n\r\n";
        server.sendContent(response);
        server.sendContent((const char *)fb->buf, fb->len);
        server.sendContent("\r\n");

        esp_camera_fb_return(fb);
    }

    // El cliente se ha desconectado
    Serial.println("Client disconnected");
}

void startServer() {
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/plain", "ESP32-CAM IP Camera");
    });

    server.on("/stream", HTTP_GET, handle_jpg_stream);
    server.begin();
    Serial.println("HTTP server started");
}

void connectToWiFi() {
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println(" connected");
    Serial.print("Camera Stream Ready! Go to: http://");
    Serial.print(WiFi.localIP());
    Serial.println("/stream");
}

void reconnectWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi connection lost. Reconnecting...");
        connectToWiFi();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    StartCamera();
    connectToWiFi();
    startServer();
}

void loop() {
    server.handleClient();
    reconnectWiFi(); // Verifica el estado de la conexión WiFi en cada iteración
}
