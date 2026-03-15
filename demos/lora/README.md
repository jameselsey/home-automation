# LoRa RYLR998 Demo — Transmitter & Receiver

A YouTube demo project using two **REYAX RYLR998** LoRa modules on **ESP32 Lolin D32** boards.
Configured for **915 MHz** (Australia / US).

---

## What it does

### Transmitter
- Configures the RYLR998 via AT commands on startup (band, SF, power, etc.)
- Sends a `PING:<count>` message every second to the receiver's address
- Logs each send to Serial (115200 baud)

### Receiver
- Configures its own RYLR998 on startup
- Listens for incoming ping messages
- Displays the **last 5 pings** on a 1.8" ST7735 TFT in landscape mode
- Each ping row shows the message text, RSSI (dBm), and SNR
- Great for a walk-test — watch the RSSI drop as you move away

---

## Hardware

- 2× ESP32 Lolin D32
- 2× REYAX RYLR998 LoRa module
- 1× 1.8" ST7735 SPI TFT display (receiver only)

---

## Wiring

### Both boards — RYLR998 to ESP32

| RYLR998 Pin | ESP32 Pin       | Notes                  |
|-------------|-----------------|------------------------|
| VDD         | 3.3V            | **3.3V only — not 5V** |
| GND         | GND             |                        |
| TXD         | GPIO16 (RX2)    | Module TX → ESP32 RX   |
| RXD         | GPIO17 (TX2)    | Module RX → ESP32 TX   |
| RST         | (not connected) | Optional               |
| NRST        | (not connected) | Optional               |

> The RYLR998 is a 3.3V device. Do **not** connect to 5V.

---

### Receiver only — ST7735 TFT to ESP32

| TFT Pin | ESP32 Pin    | Notes                        |
|---------|--------------|------------------------------|
| VCC     | 3.3V         |                              |
| GND     | GND          |                              |
| CS      | GPIO5        | Chip Select                  |
| RESET   | GPIO4        | Reset                        |
| AO      | GPIO25       | Data/Command (also called DC)|
| SDA     | GPIO23       | SPI MOSI                     |
| SCK     | GPIO18       | SPI Clock                    |
| LED     | 3.3V or GPIO | Backlight (opt)              |

---

## Libraries Required

Install via Arduino Library Manager:

- `Adafruit ST7735 and ST7789 Library`
- `Adafruit GFX Library`

---

## LoRa Configuration

Both modules are configured with matching settings:

| Parameter      | Value              |
|----------------|--------------------|
| Band           | 915 MHz (AU)       |
| Network ID     | 18 (public)        |
| Spreading Factor | 9                |
| Bandwidth      | 7 (125 kHz)        |
| Coding Rate    | 1 (4/5)            |
| Preamble       | 12                 |
| TX Power       | 22 dBm (max)       |
| Transmitter address | 1             |
| Receiver address    | 2             |

To change the frequency or SF, edit the `#define` values at the top of each `.ino` file.

---

## AT Commands Used

The ESP32 sends these AT commands to the RYLR998 on boot:

```
AT+RESET
AT+ADDRESS=<n>
AT+NETWORKID=18
AT+BAND=915000000
AT+PARAMETER=9,7,1,12
AT+CRFOP=22          (transmitter only — TX power)
```

You can monitor the AT command exchange on the Serial Monitor at **115200 baud**.

---

## Range Test Tips

- Higher SF = longer range, slower data rate. SF9 is a good balance.
- The RSSI and SNR values on the display are colour coded:

| Colour | RSSI | SNR | Meaning |
|--------|------|-----|---------|
| Green  | better than -80 dBm | above 5 dB | Strong signal |
| Orange | -80 to -100 dBm | 0 to 5 dB | Usable but weakening |
| Red    | worse than -100 dBm | below 0 dB | Marginal, dropouts likely |

- The RYLR998 with a simple wire antenna can reach 1–3 km line-of-sight at SF9/22dBm.
