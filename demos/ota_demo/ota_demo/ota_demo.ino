#include <WiFi.h>         // For ESP32, replace with <ESP8266WiFi.h> if using ESP8266
#include <ArduinoOTA.h>
#include <Adafruit_GFX.h>        // Core graphics library
#include <Adafruit_ST7735.h>     // Library for ST7735
#include <SPI.h>

// Define pins for the display
#define TFT_CS         5  // Chip Select
#define TFT_RST        4  // Reset
#define TFT_DC         2  // Data/Command

const char* ssid = "your wifi";           // Replace with your WiFi SSID
const char* password = "your wifi password";   // Replace with your WiFi Password

// Initialize the ST7735 display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");



  ArduinoOTA.begin();
  Serial.println("OTA Ready");

  // Initialize the display
  tft.initR(INITR_BLACKTAB);  // Init ST7735S chip
  tft.setRotation(1);         // Set the display rotation
  
  // Set the display background color to black
  tft.fillScreen(ST77XX_BLACK);
  
  // Print a message on the display
  tft.setTextColor(ST77XX_WHITE);  // Set text color to white
  tft.setTextSize(4);              // Set text size
  tft.setCursor(0, 0);             // Set starting point for text
  
}

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();

  tft.fillScreen(ST77XX_BLACK);
  
  // Calculate width and height of the text
  int x = (160 ) / 2; // Small offset to fit within 160px width
  int y = (128 - 16) / 2; // Centered vertically
  // Print centered text
  tft.setCursor(x, y);
    tft.print("v2");
  
  delay(1000);
}
