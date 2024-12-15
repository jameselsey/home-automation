#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

#define DFPSerial Serial1

#define DFP_VOLUME 3 // from 0 to 30
#define DFP_RX_PIN 16  
#define DFP_TX_PIN 17  
#define PIR_PIN 19            // PIR sensor connected to GPIO 19

#define PLAY_SOUND_COUNT 3
#define PLAY_SOUND_DELAY 5000

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void setup()
{
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);    // Set PIR sensor pin as input

  DFPSerial.begin(9600, SERIAL_8N1, DFP_RX_PIN, DFP_TX_PIN);


  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(DFPSerial, /*isACK = */true, /*doReset = */true)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0);
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(DFP_VOLUME);
}

void loop() {
  static bool playing = false;         
  static unsigned long lastPlayTime = 0;
  static int soundsPlayed = 0;         // Counter for the number of sounds played in the current detection
  static int pirState = LOW;           // Current state of the PIR sensor
  
  int motionDetected = digitalRead(PIR_PIN); // Read PIR sensor state (HIGH or LOW)

  // If motion is detected and we're not already playing sounds
  if (motionDetected == HIGH && !playing) {
    Serial.println(F("Motion detected! Starting playback..."));
    playing = true;                    // Start the playback sequence
    soundsPlayed = 0;                  // Reset the sounds played counter
    lastPlayTime = millis();           // Reset the timer
  }

  // If in playback mode, handle the sound playback sequence
  if (playing) {
    if (soundsPlayed < PLAY_SOUND_COUNT) {            // Play up to 3 sounds
      if (millis() - lastPlayTime >= PLAY_SOUND_DELAY) { // delay between sounds
        int randomSound = random(1, 9); // Random sound between 1.mp3 and 8.mp3
        myDFPlayer.play(randomSound);
        Serial.print(F("Playing sound: "));
        Serial.println(randomSound);
        lastPlayTime = millis();      // Reset the timer
        soundsPlayed++;               // Increment the sound counter
      }
    } else {
      // All 3 sounds have been played; stop playback
      Serial.println(F("Playback complete."));
      playing = false;                // Exit playback mode
    }
  }

  // Debugging messages for DFPlayer Mini
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());
  }
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
