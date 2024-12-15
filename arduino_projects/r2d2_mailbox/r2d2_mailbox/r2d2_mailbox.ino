#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#include "esp_sleep.h"

#define DFPSerial Serial1

#define DFP_VOLUME 5 // from 0 to 30
#define DFP_RX_PIN 16  
#define DFP_TX_PIN 17  
#define PIR_PIN 2            // PIR sensor connected to GPIO 19

#define PLAY_SOUND_COUNT 3
#define PLAY_SOUND_DELAY 5000

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void playSounds() {
  Serial.println(F("Playing sounds..."));
  for (int i = 0; i < PLAY_SOUND_COUNT; i++) {
    int randomSound = random(1, 9); // Random sound between 1.mp3 and 8.mp3
    myDFPlayer.play(randomSound);
    Serial.print(F("Playing sound: "));
    Serial.println(randomSound);
    delay(PLAY_SOUND_DELAY);
  }
  Serial.println(F("Playback complete."));
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);

  // Configure PIR pin as input with internal pull-down
  pinMode(PIR_PIN, INPUT_PULLDOWN);

  // Check wakeup reason
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println(F("Woke up from PIR motion detection!"));
    // Initialize DFPlayer
    DFPSerial.begin(9600, SERIAL_8N1, DFP_RX_PIN, DFP_TX_PIN);
    if (!myDFPlayer.begin(DFPSerial)) {
      Serial.println(F("Unable to initialize DFPlayer!"));
      while (true); // Halt if DFPlayer fails
    }
    myDFPlayer.volume(DFP_VOLUME);

    // Play sounds
    playSounds();
  } else {
    Serial.println(F("Starting fresh setup."));
  }

  // Configure PIR pin as the wakeup source
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_PIN, 1); // Wake up on HIGH signal

  // Enter deep sleep
  Serial.println(F("Entering deep sleep..."));
  delay(100); // Small delay to ensure logs are printed
  esp_deep_sleep_start();
}

void loop() {
  // Deep sleep mode, so loop will not execute
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}
