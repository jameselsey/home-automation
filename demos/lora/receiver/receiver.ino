/**
 * RYLR998 LoRa Receiver with TFT Display
 * ESP32 Lolin D32
 * 
 * Receives ping messages from the transmitter and displays the last 5
 * on a 1.8" ST7735 TFT in landscape mode.
 * 
 * Wiring - RYLR998:
 *   RYLR998 VDD  -> 3.3V
 *   RYLR998 GND  -> GND
 *   RYLR998 TXD  -> GPIO16 (RX2)
 *   RYLR998 RXD  -> GPIO17 (TX2)
 * 
 * Wiring - TFT (ST7735 1.8"):
 *   TFT VCC   -> 3.3V
 *   TFT GND   -> GND
 *   TFT CS    -> GPIO5
 *   TFT RESET -> GPIO4
 *   TFT AO    -> GPIO25 (Data/Command)
 *   TFT SDA   -> GPIO23 (SPI MOSI)
 *   TFT SCK   -> GPIO18 (SPI CLK)
 *   TFT LED   -> 3.3V  (backlight, always on)
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// --- TFT pins ---
#define TFT_CS   5
#define TFT_DC   25
#define TFT_RST  4

// --- LoRa Serial2 pins ---
#define LORA_RX   16
#define LORA_TX   17
#define LORA_BAUD 115200

// LoRa config (must match transmitter)
#define LORA_ADDRESS    2
#define LORA_NETWORK    18
#define LORA_BAND       915000000
#define LORA_SF         9
#define LORA_BW         7
#define LORA_CR         1
#define LORA_PREAMBLE   12

// Display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Landscape: 160x128
#define SCREEN_W  160
#define SCREEN_H  128

// Last 5 pings
#define MAX_PINGS 5
struct PingEntry {
  String msg;
  int rssi;
  int snr;
};
PingEntry pings[MAX_PINGS];
int pingCount = 0;

HardwareSerial loraSerial(2);

// --- AT command helper ---
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
  sendAT("AT+RESET");
  delay(1000);
  sendAT("AT+ADDRESS=" + String(LORA_ADDRESS));
  sendAT("AT+NETWORKID=" + String(LORA_NETWORK));
  sendAT("AT+BAND=" + String(LORA_BAND));
  sendAT("AT+PARAMETER=" + String(LORA_SF) + "," +
         String(LORA_BW) + "," +
         String(LORA_CR) + "," +
         String(LORA_PREAMBLE));
  Serial.println("RYLR998 configured.");
}

// --- Signal quality colour helpers ---
// RSSI: green > -80, orange -80 to -100, red < -100
uint16_t rssiColor(int rssi) {
  if (rssi > -80)  return ST77XX_GREEN;
  if (rssi > -100) return 0xFD20; // orange
  return ST77XX_RED;
}

// SNR: green > 5, orange 0-5, red < 0
uint16_t snrColor(int snr) {
  if (snr > 5) return ST77XX_GREEN;
  if (snr > 0) return 0xFD20; // orange
  return ST77XX_RED;
}

// --- Display ---
void drawHeader() {
  tft.fillRect(0, 0, SCREEN_W, 16, ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(4, 4);
  tft.print("LoRa Receiver  915MHz");
}

void drawPings() {
  tft.fillRect(0, 18, SCREEN_W, SCREEN_H - 18, ST77XX_BLACK);
  tft.setTextSize(1);

  int total = min(pingCount, MAX_PINGS);
  for (int i = 0; i < total; i++) {
    int idx = (pingCount - 1 - i + MAX_PINGS) % MAX_PINGS;
    int y = 20 + i * 22;

    uint16_t rowColor = (i % 2 == 0) ? 0x1082 : ST77XX_BLACK;
    tft.fillRect(0, y - 1, SCREEN_W, 21, rowColor);

    // Message in white
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(4, y);
    tft.print(pings[idx].msg);

    // RSSI coloured by quality
    tft.setCursor(4, y + 11);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("RSSI:");
    tft.setTextColor(rssiColor(pings[idx].rssi));
    tft.print(pings[idx].rssi);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("dBm ");

    // SNR coloured by quality
    tft.print("SNR:");
    tft.setTextColor(snrColor(pings[idx].snr));
    tft.print(pings[idx].snr);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("dB");
  }

  if (pingCount == 0) {
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(4, 40);
    tft.print("Waiting for pings...");
  }
}

// Parse incoming RYLR message:
// Format: +RCV=<addr>,<len>,<data>,<rssi>,<snr>
void parseAndStore(const String& line) {
  if (!line.startsWith("+RCV=")) return;

  String payload = line.substring(5); // strip "+RCV="

  // Split by comma: addr, len, data, rssi, snr
  int c1 = payload.indexOf(',');
  int c2 = payload.indexOf(',', c1 + 1);
  int c3 = payload.lastIndexOf(',');
  int c4 = payload.indexOf(',', c3 - 10); // snr is last field

  if (c1 < 0 || c2 < 0) return;

  // Find last two commas for rssi and snr
  // Format: addr,len,data,rssi,snr
  // We need to find the 4th and 5th commas
  int commaPos[5];
  int found = 0;
  for (int i = 0; i < (int)payload.length() && found < 5; i++) {
    if (payload[i] == ',') commaPos[found++] = i;
  }
  if (found < 4) return;

  String data = payload.substring(commaPos[1] + 1, commaPos[2]);
  int rssi    = payload.substring(commaPos[2] + 1, commaPos[3]).toInt();
  int snr     = payload.substring(commaPos[3] + 1).toInt();

  int slot = pingCount % MAX_PINGS;
  pings[slot].msg  = data;
  pings[slot].rssi = rssi;
  pings[slot].snr  = snr;
  pingCount++;

  Serial.println("Received: " + data + " RSSI=" + rssi + " SNR=" + snr);
  drawPings();
}

void setup() {
  Serial.begin(115200);
  loraSerial.begin(LORA_BAUD, SERIAL_8N1, LORA_RX, LORA_TX);

  // Init display
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1); // landscape
  tft.fillScreen(ST77XX_BLACK);

  drawHeader();
  drawPings();

  Serial.println("LoRa Receiver starting...");
  configureRYLR();

  // Redraw after config (serial output may have taken time)
  drawHeader();
  drawPings();
}

void loop() {
  // Read full line from LoRa module
  if (loraSerial.available()) {
    String line = loraSerial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      Serial.println("RAW: " + line);
      parseAndStore(line);
    }
  }
}
