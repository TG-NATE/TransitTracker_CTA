# CTA Bus Tracker

A real-time Chicago bus arrival board built on an ESP32 and a 4.2" e-ink display. It pulls live predictions from the CTA Bus Tracker API for the stops you choose, groups them by route, and refreshes the screen at a configurable interval (default 180 s, constrained 60–900 s).

## How it works

1. The ESP32 connects to Wi-Fi.
2. Every cycle it calls the CTA Bus Tracker `getpredictions` endpoint for your configured stop IDs.
3. The JSON response is parsed with ArduinoJson, pulling the route, destination, direction, and minutes until arrival for each prediction.
4. Predictions are grouped by route (up to 12 routes) and drawn as left-aligned text lines, for example `22  95th Howard     S   DUE,13,24`. Up to 3 arrivals per route.
5. The display is put into deep sleep between refreshes to protect the panel, and uses the `millis()` timer to refresh the arrivals at the configured refresh interval.
6. 30s retry interval when fetching arrivals fail. 10s timeout + 30s retry interval when WiFi connection fails. Old arrivals stay on the screen after a failed fetch.

## Hardware

- ESP32 dev board
- 4.2" e-ink display, GDEY042T81 (driven through the GxEPD2 `GxEPD2_420_GDEY042T81` class)
- Jumper wires

### Wiring

| Display pin | ESP32 pin |
| --- | --- |
| CS | GPIO 5 |
| DC | GPIO 17 |
| RST | GPIO 16 |
| BUSY | GPIO 4 |
| SCK / MOSI (DIN) | Default hardware SPI pins (GPIO 18 / GPIO 23 on most ESP32 boards) |
| VCC / GND | 3.3V / GND |

## Software setup

### Requirements

- [Arduino IDE](https://www.arduino.cc/en/software) with the ESP32 board package installed
- `ESP32 core 4.x`
- Libraries (install through the Library Manager):
  - `GxEPD2`
  - `Adafruit GFX Library` (provides the fonts)
  - `ArduinoJson 7`
- A free [CTA Bus Tracker API key](https://www.transitchicago.com/developers/bustracker/)

### Configuration

config.h values are now defaults; Program reads NVS and falls back to config.h as nothing is stored there yet.

In the future values used and saved in the NVS under namespace tracker will hold ssid, password, stops, and refresh const. This will be done in the next phase as BLE setup is worked on. API Keys will remain in config.h and loaded at compile time

Create a file named `config.h` in the sketch folder with your own values:

```cpp
#ifndef CONFIG_H
#define CONFIG_H

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* busAPIKey = "YOUR_CTA_API_KEY";
const char* stopid = "YOUR_STOP_ID";  // comma-separated CTA stop IDs

#endif
```

Find stop IDs on the CTA Bus Tracker site, or from the stop's sign. `config.h` holds your Wi-Fi password and API key, so keep it out of version control (it should be listed in `.gitignore`).

#### Resetting saved settings

To reset saved setting, hold BOOT for 3 s while the board is running to clear NVS and restart. Note: holding BOOT at power-on enters download mode instead.

### Build and upload

1. Open `TransitTracker.ino` in the Arduino IDE.
2. Select your ESP32 board and port.
3. Upload, then open the Serial Monitor at 115200 baud to watch Wi-Fi status and API responses.

## Project structure

```
src/TransitTracker
  TransitTracker.ino   Main sketch: Wi-Fi, API calls, JSON parsing, display drawing
  transit_types.h      RouteGroup struct that groups Arrival by route
  config.h             Your Wi-Fi and API settings (not committed)
```

## Tech used

- C++ (Arduino framework), ESP32, JSON parsing, GxEPD2 e-ink display driver, HTTPS with the CA bundle
- NVS/Preferences: Stored in NVS so data survives reboot
- non-blocking timing: used `millis()` to stop the whole program from being blocked

## License
Released under the GPL-3.0 license. See `LICENSE` for details.


