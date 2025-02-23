#include <Wire.h>
#include <ArduinoOTA.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <HTTPClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <PubSubClient.h>

// #include <Adafruit_Sensor.h>
// #include "utils.h"

// Pin wiring:
//    VCC -- 3v
//    GND -- GND
//    SCL -- 22
//    SDA -- 21


#define SEALEVELPRESSURE_HPA (961.77) // Replace with the actual sea level pressure your area

Adafruit_BMP280 bmp; // I2C

struct Config {
  String thingName;
  String wifiSsid;
  String wifiPassword;
  String mqttTopic;
  String mqttCommandTopic;
  String mqttServer;
  uint16_t mqttPort;
};

Config config;

// Define MQTT variables
WiFiClient espClient;
PubSubClient mqttClient(espClient);

unsigned long lastTemperatureReading = 0;
const int temperatureReadingInterval = 60000;  // Temperature reading interval in milliseconds

void setup() {
  Serial.begin(9600);
  
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

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
  
  // Setup MQTT Client
  mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
  mqttClient.setCallback(mqttCallback);
  

  Serial.println("Started device " + config.thingName);
}

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();

  // Reconnect MQTT if disconnected
  if (!mqttClient.connected()) {
      Serial.println("mqtt was disconnected, attempting to reconnect");
      reconnectMQTT();
  }
  mqttClient.loop();
  
  // Take temperature reading every 30 seconds
  if (millis() - lastTemperatureReading > temperatureReadingInterval) {

    Serial.println("Reading BMP280...");

    float temperature = bmp.readTemperature();
    float pressure = bmp.readPressure() / 100.0F;

    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" hPa");

    float altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
    Serial.print("Approx. Altitude = ");
    Serial.print(altitude);
    Serial.println(" m");

    Serial.println();

    publishMessage(temperature, pressure);  

    lastTemperatureReading = millis();
  }
}

void publishMessage(float temperature, float pressure){
  publishMqtt(temperature, pressure);
}

void publishMqtt(float temperature, float pressure) {
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

  // Create the JSON payload
  StaticJsonDocument<256> jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["pressure"] = pressure;
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Publish to MQTT topic
  mqttClient.publish(config.mqttTopic.c_str(), jsonString.c_str());

  Serial.println("Completed publishing update");
}

void reconnectMQTT() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Reconnecting...");
        return;
    }

    mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);

    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT Broker at ");
        Serial.print(config.mqttServer);
        Serial.print(":");
        Serial.println(config.mqttPort);

        if (mqttClient.connect(config.thingName.c_str())) {
            Serial.println("✅ Connected to MQTT!");

            // Subscribe to topics
            mqttClient.subscribe(config.mqttCommandTopic.c_str());
            Serial.println("📡 Subscribed to: " + config.mqttCommandTopic);
        } else {
            Serial.print("❌ MQTT Connection failed! Error Code: ");
            Serial.println(mqttClient.state());

            // Print detailed error message
            switch (mqttClient.state()) {
                case -4: Serial.println("MQTT_CONNECTION_TIMEOUT"); break;
                case -3: Serial.println("MQTT_CONNECTION_LOST"); break;
                case -2: Serial.println("MQTT_CONNECT_FAILED"); break;
                case -1: Serial.println("MQTT_DISCONNECTED"); break;
                case  1: Serial.println("MQTT_CONNECT_BAD_PROTOCOL"); break;
                case  2: Serial.println("MQTT_CONNECT_BAD_CLIENT_ID"); break;
                case  3: Serial.println("MQTT_CONNECT_UNAVAILABLE"); break;
                case  4: Serial.println("MQTT_CONNECT_BAD_CREDENTIALS"); break;
                case  5: Serial.println("MQTT_CONNECT_UNAUTHORIZED"); break;
                default: Serial.println("Unknown error");
            }

            delay(5000);
        }
    }
}


// MQTT Callback function
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.println(topic);

    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.print("Message: ");
    Serial.println(message);

    // Example: If receiving "restart" command
    if (String(topic) == config.mqttCommandTopic && message == "restart") {
        Serial.println("Restart command received. Restarting ESP...");
        ESP.restart();
    }
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

  if (jsonDoc.containsKey("mqttTopic")) {
    config.mqttTopic = String((const char*)jsonDoc["mqttTopic"]);
  } else {
    Serial.println("mqttTopic not found in config.json");
  }

  if (jsonDoc.containsKey("mqttCommandTopic")) {
    config.mqttCommandTopic = String((const char*)jsonDoc["mqttCommandTopic"]);
  } else {
    Serial.println("mqttCommandTopic not found in config.json");
  }

  if (jsonDoc.containsKey("mqttServer")) {
    config.mqttServer = String((const char*)jsonDoc["mqttServer"]);
  } else {
    Serial.println("mqttServer not found in config.json");
  }

  if (jsonDoc.containsKey("mqttPort")) {
    config.mqttPort = (uint16_t) jsonDoc["mqttPort"].as<unsigned int>(); 
  } else {
    Serial.println("mqttPort not found in config.json");
  } 
  return config;
}
