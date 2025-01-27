#include <Wire.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <HTTPClient.h>
// #include <Adafruit_Sensor.h>
// #include "utils.h"

// Pin wiring:
//    VCC -- 3v
//    GND -- GND
//    SCL -- 22
//    SDA -- 21

const int reedSwitchPin1 = 15;  // Define the digital pin connected to the reed switch
int reedSwitch1State = 0;
const String doorName = "pa-door";

struct Config {
  String thingName;
  String wifiSsid;
  String wifiPassword;
  String discordWebhookUrl;
};

Config config;

void setup() {
  Serial.begin(9600);
  

  pinMode(reedSwitchPin1, INPUT_PULLUP);  // Set the reed switch pin as an input with internal pull-up resistor
  reedSwitch1State = digitalRead(reedSwitchPin1); // set the initial state of the reed switch
  
  // Load configuration
  config = loadConfig();

  // Connect to wifi
  WiFi.disconnect(true);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // Resets the default hostname
  WiFi.setHostname(config.thingName.c_str());

  Serial.println("Connecting to WiFi");

  WiFi.begin(config.wifiSsid, config.wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Start the OTA service, so it knows to listen for updates
  ArduinoOTA.setHostname(config.thingName.c_str());
  ArduinoOTA.begin();
  
  
  

  Serial.println("Started device " + config.thingName);
}

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();

  // check reed switch
  int reedSwitchState1Loop = digitalRead(reedSwitchPin1);  // Read the state of the reed switch

  // if the door was closed, but now opened
  if (reedSwitch1State == LOW && reedSwitchState1Loop == HIGH){
    Serial.println("Door opened");
    publishDoorTransition("OPEN");
    reedSwitch1State = reedSwitchState1Loop;
  }
   else if (reedSwitch1State == HIGH && reedSwitchState1Loop == LOW) {
    Serial.println("Door closed");
    publishDoorTransition("CLOSE");
    reedSwitch1State = reedSwitchState1Loop;
   }

  // // Handle MQTT events
  // if (!client.connected()) {
  //   connectAWS();
  // }
  // client.loop();
}

void publishDoorTransition(String newState){
  sendDiscordMessage(newState);
}

void sendDiscordMessage(String transitionValue) {
    if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.disconnect(true);
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // Resets the default hostname
    WiFi.setHostname(config.thingName.c_str());
    WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Reconnected to WiFi");
  }

  HTTPClient http;



  http.begin(config.discordWebhookUrl); // Automatically handles certificates
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  http.setReuse(false);
  // Create the JSON payload
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["content"] = "Door [" + config.thingName + "] has become [" + transitionValue + "]";
  jsonDoc["transition"] = transitionValue;
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    Serial.printf("Response: %d\n", httpResponseCode);
    String response = http.getString();
    Serial.println(response);
  } else {
    Serial.printf("Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();

  Serial.println("Completed message to discord");
}

Config loadConfig() {
  Config config;

  // Initialize LittleFS
  if (!LittleFS.begin(true, "/littlefs", 10, "littlefs")) {
    Serial.println("Failed to mount LittleFS");
    return config; // Return empty Config object
  }

  // Open the config.json file
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config.json");
    return config; // Return empty Config object
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
    return config; // Return empty Config object
  }

  // Extract and set the values in the Config object
  if (jsonDoc.containsKey("thingName")) {
    config.thingName = String((const char*)jsonDoc["thingName"]);
  } else {
    Serial.println("thingName not found in config.json");
  }

  if (jsonDoc.containsKey("wifiSsid")) {
    config.wifiSsid = String((const char*)jsonDoc["wifiSsid"]);
  } else {
    Serial.println("wifiSsid not found in config.json");
  }

  if (jsonDoc.containsKey("wifiPassword")) {
    config.wifiPassword = String((const char*)jsonDoc["wifiPassword"]);
  } else {
    Serial.println("wifiPassword not found in config.json");
  }

  if (jsonDoc.containsKey("discordWebhookUrl")) {
    config.discordWebhookUrl = String((const char*)jsonDoc["discordWebhookUrl"]);
  } else {
    Serial.println("discordWebhookUrl not found in config.json");
  }

  return config;
}
