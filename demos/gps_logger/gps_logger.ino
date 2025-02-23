#include <TinyGPS++.h>
#include <HardwareSerial.h>

// Define GPS Serial
HardwareSerial gpsSerial(1); // Using UART1
TinyGPSPlus gps;

// Pin definitions for ESP32
#define RXPin 16  // GPS TX to ESP32 RX
#define TXPin 17  // GPS RX to ESP32 TX
#define GPSBaud 9600  // GPS baud rate

void setup() {
  Serial.begin(115200);  // Start serial for ESP32 console output
  gpsSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);  // Begin serial for GPS
  
  Serial.println("GPS Module Test Starting...");
}

void loop() {
  // Continuously read data from the GPS module
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    gps.encode(c);  // Parse the incoming data

    // If valid GPS data is available, print latitude and longitude
    if (gps.location.isUpdated()) {
      Serial.println(
        String(gps.date.value()) + " " +
        String(gps.time.value()) + " " +
        "lat/lon: " + 
        String(gps.location.lat(), 6) + "," + 
        String(gps.location.lng(), 6) + " " +
        "altitude: " + 
        String(gps.altitude.meters()) + "m " + 
        "speed: " + 
        String(gps.speed.kmph()) + " km/h " + 
        "heading(degrees): " + 
        String(gps.course.deg())
      );
    }
  }
  
  // Add a small delay to avoid flooding the serial output
  delay(1000);
}
