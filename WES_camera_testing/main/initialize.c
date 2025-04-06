#include "esp_camera.h"
#include <esp_wifi.h>
#include <ESPAsyncWebServer.h>

// WiFi credentials
const char *ssid = "BIRD-Incubator-WIFI";
const char *password = "BIRD01012021";

// Camera pin configuration
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Web Server
AsyncWebServer server(80);

// Function for initializing the camera
void init_camera()
{
    camera_config_t config = {
        .pin_pwdn = PWDN_GPIO_NUM,
        .pin_reset = RESET_GPIO_NUM,
        .pin_xclk = XCLK_GPIO_NUM,
        .pin_sscb_sda = SIOD_GPIO_NUM,
        .pin_sscb_scl = SIOC_GPIO_NUM,
        .pin_d7 = Y9_GPIO_NUM,
        .pin_d6 = Y8_GPIO_NUM,
        .pin_d5 = Y7_GPIO_NUM,
        .pin_d4 = Y6_GPIO_NUM,
        .pin_d3 = Y5_GPIO_NUM,
        .pin_d2 = Y4_GPIO_NUM,
        .pin_d1 = Y3_GPIO_NUM,
        .pin_d0 = Y2_GPIO_NUM,
        .pin_vsync = VSYNC_GPIO_NUM,
        .pin_href = HREF_GPIO_NUM,
        .pin_pclk = PCLK_GPIO_NUM,
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_QVGA,
        .jpeg_quality = 12,
        .fb_count = 1};

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);                  // Flip the image vertically
    s->set_brightness(s, 1);             // Increase brightness slightly
    s->set_saturation(s, -2);            // Lower saturation
    s->set_framesize(s, FRAMESIZE_QVGA); // Use lower frame size for streaming
}

// WiFi connection function
void connectWiFi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

// Start the camera server for streaming
void startCameraServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      request->send(500, "text/plain", "Camera capture failed");
      return;
    }
    request->send_P(200, "image/jpeg", fb->buf, fb->len);
    esp_camera_fb_return(fb); });

    server.begin();
}

// FreeRTOS task to handle image capture and streaming
void captureAndStreamTask(void *pvParameters)
{
    while (1)
    {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // Here you can process or stream the image if needed
        Serial.printf("Captured image: %zu bytes\n", fb->len);

        // Return the frame buffer
        esp_camera_fb_return(fb);

        // Delay to prevent overloading the system
        vTaskDelay(pdMS_TO_TICKS(3000)); // capture every 3 seconds
    }
}

void setup()
{
    // Start serial communication
    Serial.begin(115200);
    Serial.println();

    // Initialize camera
    init_camera();

    // Connect to WiFi
    connectWiFi();

    // Start the camera server for streaming
    startCameraServer();

    // Create FreeRTOS task for image capture and streaming
    xTaskCreate(captureAndStreamTask, "captureAndStreamTask", 8192, NULL, 1, NULL);
}

void loop()
{
    // Do nothing, camera streaming and image capture handled by FreeRTOS tasks
    delay(10000);
}
