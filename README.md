# CTA Bus Tracker

A real-time Chicago bus arrival board built on an ESP32 and a 4.2" e-ink display. It pulls live predictions from the CTA Bus Tracker API for the stops you choose, groups them by route, and refreshes the screen every 3 minutes.

## How it works

1. The ESP32 connects to Wi-Fi.
2. Every cycle it calls the CTA Bus Tracker `getpredictions` endpoint for your configured stop IDs (up to 4 predictions per request).
3. The JSON response is parsed with ArduinoJson, pulling the route, destination, direction, and minutes until arrival for each prediction.
4. Predictions are grouped by route (up to 12 routes) and drawn as centered text lines, for example `R22 Howard Southbound 5m`. Buses about to arrive show `DUE`.
5. The display is put into deep sleep between refreshes to protect the panel, and the loop waits 3 minutes before the next update.

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
- Libraries (install through the Library Manager):
  - `GxEPD2`
  - `Adafruit GFX Library` (provides the fonts)
  - `ArduinoJson`
- A free [CTA Bus Tracker API key](https://www.transitchicago.com/developers/bustracker/)

### Configuration

Create a file named `config.h` in the sketch folder with your own values:

```cpp
#ifndef CONFIG_H
#define CONFIG_H

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
const char* busAPIKey = "YOUR_CTA_API_KEY";
const char* stopid = "1593,18318";  // comma-separated CTA stop IDs

#endif
```

Find stop IDs on the CTA Bus Tracker site, or from the stop's sign. `config.h` holds your Wi-Fi password and API key, so keep it out of version control (it should be listed in `.gitignore`).

### Build and upload

1. Open `TransitTracker.ino` in the Arduino IDE.
2. Select your ESP32 board and port.
3. Upload, then open the Serial Monitor at 115200 baud to watch Wi-Fi status and API responses.

## Project structure

```
src/
  TransitTracker.ino   Main sketch: Wi-Fi, API calls, JSON parsing, display drawing
  transit_types.h      RouteTracker struct that groups arrivals by route
  config.h             Your Wi-Fi and API settings (not committed)
```

## Tech used

C++ (Arduino framework), ESP32, HTTP/REST API calls, JSON parsing, GxEPD2 e-ink display driver.

<!--## License-->
<!---->
<!--Released under the GPL-3.0 license. See `LICENSE` for details.-->

