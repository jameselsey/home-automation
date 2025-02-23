#include <SPI.h>
#include <SD.h>

const int chipSelect = 21; // Set to the pin you've used for CS (chip select)

void setup() {
  Serial.begin(115200);

  // Initialize the SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Open file for writing
  File myFile = SD.open("/example.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("Writing to example.txt...");

    // Write 10 lines of text to the file
    for (int i = 1; i <= 10; i++) {
      myFile.print("This is line ");
      myFile.println(i);
    }

    // Close the file
    myFile.close();
    Serial.println("Done writing.");
  } else {
    // If the file didn't open, print an error:
    Serial.println("Error opening file for writing.");
  }
}

void loop() {
  // Nothing to do here
}
