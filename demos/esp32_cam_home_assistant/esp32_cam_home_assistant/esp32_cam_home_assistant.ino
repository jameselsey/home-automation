#include <WiFi.h>
#include "esp_camera.h"
#include "esp_sleep.h"

// Wi-Fi credentials
const char* ssid = "wifi name";
const char* password = "wifi password";

// Server info
String serverName = "192.168.0.13"; // Use mDNS or IP
const int serverPort = 5001;
String serverPath = "/mailbox";

// Deep sleep setup
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 30  // in seconds

WiFiClient client;

// Camera pin config for AI-Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection failed");
  }
}

void setupCamera() {
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
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x", err);
    delay(2000);
    ESP.restart();
  }
}

bool sendImage(camera_fb_t *fb) {
  Serial.println("Connecting to server...");

  if (!client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection failed");
    return false;
  }

  Serial.println("Connection successful!");

  // Send HTTP POST headers
  client.println("POST " + serverPath + " HTTP/1.1");
  client.println("Host: " + serverName);
  client.println("Content-Type: application/octet-stream");
  client.println("Content-Length: " + String(fb->len));
  client.println();
  
  // Send the image data
  client.write(fb->buf, fb->len);

  // Release camera buffer
  esp_camera_fb_return(fb);

  // Read response
  String response;
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
      response += c;
    }
  }

  client.stop();
  Serial.println("Server response:");
  Serial.println(response);

  return true;
}


void setup() {
  Serial.begin(115200);
  delay(100);

  connectWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Sleeping...");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }

  setupCamera();

  // Optional: turn on LED flash
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  delay(200); // warm-up
  camera_fb_t *fb = esp_camera_fb_get();
  digitalWrite(4, LOW); // turn off flash

  if (!fb) {
    Serial.println("Camera capture failed");
  } else {
    if (sendImage(fb)) {
      Serial.println("Image sent successfully");
    } else {
      Serial.println("Image failed to send");
    }
    esp_camera_fb_return(fb);
  }

  // Cleanup
  esp_camera_deinit();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  Serial.println("Going to deep sleep...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // never used
}
