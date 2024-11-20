// libs for littlefs
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

// libs for tft
#include <Adafruit_GFX.h>        // Core graphics library
#include <Adafruit_ST7735.h>     // Library for ST7735
#include <SPI.h>

// Define pins for the display
#define TFT_CS         5  // Chip Select
#define TFT_RST        4  // Reset
#define TFT_DC         2  // Data/Command

// Initialize the ST7735 display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  // Initialize LittleFS
  if (!LittleFS.begin(true, "/littlefs", 10, "littlefs")) {
    Serial.println("Failed to mount LittleFS");
    return;
  }

  // Open the config.json file
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config.json");
    return;
  }

  // Read the file contents into a string
  String fileContent = "";
  while (configFile.available()) {
    fileContent += char(configFile.read());
  }
  configFile.close();

  // Parse the JSON
  StaticJsonDocument<512> jsonDoc; // Adjust the size if needed
  DeserializationError error = deserializeJson(jsonDoc, fileContent);
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Extract and print the value of "thingName"
  const char* thingName = jsonDoc["thingName"];
  if (thingName) {
    Serial.print("thingName: ");
    Serial.println(thingName);
  } else {
    Serial.println("thingName not found in config.json");
  }


  // Initialize the display
  tft.initR(INITR_BLACKTAB);  // Init ST7735S chip
  tft.setRotation(1);         // Set the display rotation
  
  // Set the display background color to black
  tft.fillScreen(ST77XX_BLACK);
  
  // Print a message on the display
  tft.setTextColor(ST77XX_WHITE);  
  tft.setTextSize(3);              
  tft.setCursor(0, 0);             
  tft.println(thingName);
  tft.println("v1");

}

void loop() {
  // Nothing to do here
}
