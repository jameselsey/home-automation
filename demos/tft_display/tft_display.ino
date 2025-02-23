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
  // Initialize the Serial Monitor
  Serial.begin(115200);
  
  // Initialize the display
  tft.initR(INITR_BLACKTAB);  // Init ST7735S chip
  tft.setRotation(1);         // Set the display rotation
  
  // Set the display background color to black
  tft.fillScreen(ST77XX_BLACK);
  
  // Print a message on the display
  tft.setTextColor(ST77XX_WHITE);  // Set text color to white
  tft.setTextSize(1);              // Set text size
  tft.setCursor(0, 0);             // Set starting point for text
  tft.println("Hello, ESP32!");
  
  // Simulate GPS Data on the display
  float lat = 37.7749;
  float lon = -122.4194;
  float altitude = 30.0;
  
  // Print data
  tft.setCursor(0, 20);            // Move cursor down
  tft.println("GPS Data:");
  tft.print("Lat: ");
  tft.println(lat, 6);
  tft.print("Lon: ");
  tft.println(lon, 6);
  tft.print("Alt: ");
  tft.print(altitude);
  tft.println(" m");
  Serial.println("started up!");
}

void loop() {
  // You can update the display data here
}
