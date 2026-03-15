/**
 * RYLR998 LoRa Transmitter
 * ESP32 Lolin D32
 * 
 * Sends a ping message every second via the RYLR998 module using AT commands.
 * The RYLR998 is controlled over UART (Serial2).
 * 
 * Wiring:
 *   RYLR998 VDD  -> 3.3V
 *   RYLR998 GND  -> GND
 *   RYLR998 TXD  -> GPIO16 (RX2)
 *   RYLR998 RXD  -> GPIO17 (TX2)
 *   RYLR998 RST  -> (optional) GPIO4
 */

#include <Arduino.h>

// Serial2 pins
#define LORA_RX   16
#define LORA_TX   17
#define LORA_BAUD 115200

// LoRa config
#define LORA_ADDRESS    1       // This device address
#define LORA_DEST       2       // Receiver address
#define LORA_NETWORK    18      // Network ID (0-16, 18 = public)
#define LORA_BAND       915000000
#define LORA_SF         9       // Spreading factor (7-12)
#define LORA_BW         7       // Bandwidth (7 = 125kHz)
#define LORA_CR         1       // Coding rate (1 = 4/5)
#define LORA_PREAMBLE   12
#define LORA_POWER      22      // TX power dBm (max 22)

HardwareSerial loraSerial(2);

uint32_t pingCount = 0;

// Send an AT command and wait for response
String sendAT(const String& cmd, uint32_t timeout = 1000) {
  loraSerial.println(cmd);
  String response = "";
  uint32_t start = millis();
  while (millis() - start < timeout) {
    while (loraSerial.available()) {
      response += (char)loraSerial.read();
    }
    if (response.indexOf("+OK") >= 0 || response.indexOf("+ERR") >= 0) break;
  }
  response.trim();
  Serial.println(">> " + cmd);
  Serial.println("<< " + response);
  return response;
}

void configureRYLR() {
  delay(500);

  // Soft reset
  sendAT("AT+RESET");
  delay(1000);

  // Set address
  sendAT("AT+ADDRESS=" + String(LORA_ADDRESS));

  // Set network ID
  sendAT("AT+NETWORKID=" + String(LORA_NETWORK));

  // Set band (915 MHz for Australia)
  sendAT("AT+BAND=" + String(LORA_BAND));

  // Set RF parameters: SF, BW, CR, preamble
  sendAT("AT+PARAMETER=" + String(LORA_SF) + "," +
         String(LORA_BW) + "," +
         String(LORA_CR) + "," +
         String(LORA_PREAMBLE));

  // Set TX power
  sendAT("AT+CRFOP=" + String(LORA_POWER));

  Serial.println("RYLR998 configured.");
}

void setup() {
  Serial.begin(115200);
  loraSerial.begin(LORA_BAUD, SERIAL_8N1, LORA_RX, LORA_TX);
  delay(500);

  Serial.println("LoRa Transmitter starting...");
  configureRYLR();
}

void loop() {
  pingCount++;

  // Build message: "PING:0001"
  String msg = "PING:" + String(pingCount);

  // AT+SEND=<address>,<length>,<data>
  String cmd = "AT+SEND=" + String(LORA_DEST) + "," +
               String(msg.length()) + "," + msg;

  sendAT(cmd, 500);

  Serial.println("Sent: " + msg);
  delay(1000);
}
