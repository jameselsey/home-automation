#include <TinyGPS++.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include "wifi_config.h"  // Include WiFi credentials

// GPS Setup (using HardwareSerial1 for GPS)
HardwareSerial gpsSerial(1);  // UART1 for GPS (TX/RX)
TinyGPSPlus gps;

// GPS Pin definitions for ESP32
#define GPS_RX_PIN 16  // GPS TX -> ESP32 RX (GPIO 16)
#define GPS_TX_PIN 17  // GPS RX -> ESP32 TX (GPIO 17)
#define GPSBaud 9600   // GPS baud rate

// TFT Display Setup
#define TFT_CS    5  // Chip select control pin
#define TFT_RST   4  // Reset pin
#define TFT_DC    2  // Data/command pin

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Switch Pin Setup
#define SWITCH_PIN 19  // GPIO 19 for the physical switch

// Variables to store previous values
double lastLatitude = 0;
double lastLongitude = 0;
double lastAltitude = 0;
double lastSpeed = 0;

// Variables to track the current GPS and WiFi status
bool previousGpsAvailable = false;
bool previousWifiConnected = false;
bool previousRecordingState = false;

// Variable to track if recording is on or off
bool isRecording = false;

// Function to display the status bar with icons beside their respective text
void displayStatusBar(bool gpsAvailable, bool wifiConnected, bool recordingOn) {
  // Clear the top portion for the status bar
  tft.fillRect(0, 0, 160, 20, ST77XX_BLACK);  // Status bar area
  
  // GPS Status
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(5, 5);  // Set cursor for GPS
  tft.print("GPS: ");
  
  // GPS icon next to text
  if (gpsAvailable) {
    // GPS available: Green square
    tft.fillRect(40, 5, 10, 10, ST77XX_GREEN);
  } else {
    // GPS not available: Red square
    tft.fillRect(40, 5, 10, 10, ST77XX_RED);
  }

  // WiFi Status
  tft.setCursor(70, 5);  // Set cursor for WiFi
  tft.print("WiFi: ");
  
  // WiFi icon next to text
  if (wifiConnected) {
    // WiFi connected: Green square
    tft.fillRect(110, 5, 10, 10, ST77XX_GREEN);
  } else {
    // WiFi not connected: Red square
    tft.fillRect(110, 5, 10, 10, ST77XX_RED);
  }

  // Recording Status
  tft.setCursor(120, 5);
  tft.print("REC: ");
  if (recordingOn) {
    tft.fillRect(150, 5, 10, 10, ST77XX_GREEN);  // Recording on
  } else {
    tft.fillRect(150, 5, 10, 10, ST77XX_RED);    // Recording off
  }
}

// Function to check if WiFi is connected
bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  
  // Initialize the GPS Serial
  gpsSerial.begin(GPSBaud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  
  // Initialize the TFT display
  tft.initR(INITR_BLACKTAB);    // Initialize ST7735 display
  tft.setRotation(1);           // Set rotation to landscape
  tft.fillScreen(ST77XX_BLACK); // Clear screen to black

  // Display startup message
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0, 30);
  tft.println("Waiting for GPS data...");

  // Initialize the switch pin
  pinMode(SWITCH_PIN, INPUT_PULLUP);  // Use internal pull-up resistor

  // Connect to WiFi (credentials are in wifi_config.h)
  WiFi.begin(ssid, password);
  
}

void loop() {
  // Continuously read data from GPS module
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);  // Process GPS data
  }

  // Check if GPS location is available
  bool gpsAvailable = gps.location.isValid();
  
  // Check if WiFi is connected
  bool wifiConnected = isWiFiConnected();

  // Check if the recording switch is on
  isRecording = digitalRead(SWITCH_PIN) == LOW;  // Assuming the switch pulls LOW when on


  // Update the status bar only when GPS or WiFi status changes
  if (gpsAvailable != previousGpsAvailable || wifiConnected != previousWifiConnected || isRecording != previousRecordingState) {
    displayStatusBar(gpsAvailable, wifiConnected, isRecording);
    
    // Update previous values to avoid unnecessary updates
    previousGpsAvailable = gpsAvailable;
    previousWifiConnected = wifiConnected;
    previousRecordingState = isRecording;
  }

  // Only update the display and log to Serial if GPS data has changed
  if (gpsAvailable && isRecording) {
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    double altitude = gps.altitude.meters();
    double speed = gps.speed.kmph();
    double heading = gps.course.deg();
    
    // Get the current time
    String timeString = String(gps.time.hour()) + ":" +
                        String(gps.time.minute()) + ":" +
                        String(gps.time.second());

    // Log to Serial Monitor
    Serial.print("Time: ");
    Serial.print(timeString);
    Serial.print(" - Lat: ");
    Serial.print(latitude, 6);
    Serial.print(", Lon: ");
    Serial.print(longitude, 6);
    Serial.print(", Alt: ");
    Serial.print(altitude);
    Serial.print(" m, Speed: ");
    Serial.print(speed);
    Serial.print(" km/h, Heading: ");
    Serial.println(heading);

    // Update the screen only if values have changed to reduce flicker
    if (latitude != lastLatitude || longitude != lastLongitude || altitude != lastAltitude || speed != lastSpeed) {
      
      // Clear only the area where GPS data is displayed
      tft.fillRect(0, 20, 160, 120, ST77XX_BLACK);

      // Display GPS data
      tft.setCursor(0, 30);
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(1);
      tft.println("GPS Data:");
      tft.print("Lat: ");
      tft.println(latitude, 6);
      tft.print("Lon: ");
      tft.println(longitude, 6);
      tft.print("Alt: ");
      tft.print(altitude);
      tft.println(" m");
      tft.print("Speed: ");
      tft.print(speed);
      tft.println(" km/h");
      tft.print("Heading: ");
      tft.print(heading);
      tft.println(" deg");

      // Update last known values
      lastLatitude = latitude;
      lastLongitude = longitude;
      lastAltitude = altitude;
      lastSpeed = speed;
    }

    // Add a delay to slow down screen updates
    delay(1000);  // Update every second
  }
}
