#include <Arduino.h>
#include <WiFi.h>

//Eink Libs
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include "transit_types.h"
#include "wifi_manager.h"
#include "settings.h"
#include "cta_client.h"

// --- DISPLAY HARDWARE CONSTRUCTOR MAP ---
// Maps to the pin layout: CS=5, DC=17, RST=16, BUSY=4
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(/*CS=*/5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

RouteGroup RetrievedRoutes[MAX_ROUTES] = {};

constexpr unsigned long RETRY_INTERVAL_MS = 30000UL;
constexpr unsigned long TIMEOUT_INTERVAL_MS = 10000UL;
unsigned long lastRefreshMs = 0;
unsigned long nextWaitMs = 0;

Settings settings;

void setup() {
  Serial.begin(115200);
  pinMode(BOOT_PIN, INPUT_PULLUP);

  delay(1000);

  //Load Settings
  settings = loadSettings();


  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(settings.ssid);

  display.init(115200, true, 2, false);

  bootUpText();

  display.hibernate();  // Puts the display into deep sleep to protect its elements

  if (!ensureWiFi(settings.ssid.c_str(), settings.password.c_str(), 15000)) {
    Serial.println("Offline Mode. Will try to reconnect in loop");
  } else {
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void bootUpText() {
  char line[40];
  snprintf(line, sizeof(line), "Connecting to WIFI");


  display.setRotation(0);  // Sets landscape orientation
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(line, 0, 0, &tbx, &tby, &tbw, &tbh);

  // Center text inside the bounding window boundaries
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(line);
  } while (display.nextPage());  // Executes the dynamic paging drawing loops
}


void drawArrivals() {
  display.init(115200, false, 2, false);  // false: not the first boot, avoid extra flash
  display.setRotation(0);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    uint16_t y = 15;

    for (const RouteGroup& group : RetrievedRoutes) {
      if (group.arrivalCount == 0) continue;

      char line[40];
      int len = snprintf(line, sizeof(line), "%-3.3s %-16.16s %4.1s",
                         group.route.c_str(),
                         group.destination.c_str(),
                         group.direction.c_str());

      for (int i = 0; i < group.arrivalCount; i++) {
        if (len >= ((int)sizeof(line))) break;

        const Arrival& a = group.arrivals[i];
        const char* sep = (i > 0) ? "," : "   ";

        if (a.isDue) {
          len += snprintf(line + len, sizeof(line) - len, "%sDUE", sep);
        } else {
          len += snprintf(line + len, sizeof(line) - len, "%s%d", sep, a.countdown);
        }
      }

      Serial.println(line);
      display.setCursor(5, y);
      display.print(line);
      y += 20;
    }

  } while (display.nextPage());

  display.hibernate();
}

void loop() {
  bool waitMet = millis() - lastRefreshMs >= nextWaitMs;

  if (resetHeld()) factoryReset();

  if (waitMet) {
    if (!ensureWiFi(settings.ssid.c_str(), settings.password.c_str(), 10000)) {
      Serial.println("Offline, skipping fetch");
      lastRefreshMs = millis();
      nextWaitMs = RETRY_INTERVAL_MS + TIMEOUT_INTERVAL_MS;
    } else if (!getArrivalTimes(RetrievedRoutes, settings.busAPIKey.c_str(), settings.stopIds.c_str())) {
      Serial.println("Fetch failed. Keeping current screen");
      lastRefreshMs = millis();
      nextWaitMs = RETRY_INTERVAL_MS;
    } else {
      drawArrivals();
      lastRefreshMs = millis();
      nextWaitMs = settings.refreshSec * 1000UL;
    }

    Serial.print("Will check again in ");
    Serial.print(nextWaitMs / 1000UL);
    Serial.println(" s");
  }
}