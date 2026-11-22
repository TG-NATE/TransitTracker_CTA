#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <unordered_map>
#include <string>

//Eink Libs
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include "config.h"
#include "transit_types.h"

// --- DISPLAY HARDWARE CONSTRUCTOR MAP ---
// Maps to the pin layout: CS=5, DC=17, RST=16, BUSY=4
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(/*CS=*/5, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

RouteGroup RetrievedRoutes[MAX_ROUTES] = {};
std::unordered_map<std::string, int> busIndexMap;


void setup() {
  Serial.begin(115200);
  // pinMode(5, OUTPUT);  // set the LED pin mode

  delay(1000);


  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  display.init(115200, true, 2, false);

  bootUpText();

  display.hibernate();  // Puts the display into deep sleep to protect its elements

  WiFi.begin(ssid, password);



  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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

Arrival parseArrival(const char* countdown) {
  Arrival a;

  a.isDue = (strcmp(countdown, "DUE") == 0);
  a.countdown = a.isDue ? 0 : atoi(countdown);
  return a;
}


void getArrivalTimes(RouteGroup Routes[]) {
  HTTPClient http;

  //Use Mozila root CA Bundle to verify CTA Bus Tracker API link. ESP32 core 4 refuses connection with out this.
  http.useBuiltinCACertBundle();
  http.begin(String("https://www.ctabustracker.com/bustime/api/v3/getpredictions?key=") + busAPIKey + "&stpid=" + stopid + "&top=10&format=json");

  int httpCode = http.GET();
  Serial.println(httpCode);

  if (httpCode == HTTP_CODE_OK) {
    Serial.print("Success:");
    Serial.println(httpCode);

    Serial.println("Downloading Payload");
    String payload = http.getString();
    http.end();

    JsonDocument filter;
    filter["bustime-response"]["prd"][0]["rt"] = true;
    filter["bustime-response"]["prd"][0]["des"] = true;
    filter["bustime-response"]["prd"][0]["prdctdn"] = true;
    filter["bustime-response"]["prd"][0]["rtdir"] = true;
    filter["bustime-response"]["error"][0]["msg"] = true;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));

    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
      Serial.println("Previous call data will be used if stored");

    } else {
      if (!doc["bustime-response"]["prd"].is<JsonArray>()){
        Serial.println(doc["bustime-response"]["error"][0]["msg"] | " ");
        Serial.println("Previous call data will be used if stored");
        return;
      }

      //Clean Route array
      for (int i = 0; i < MAX_ROUTES; i++) {
        Routes[i] = RouteGroup();
      }

      //Clear index map.
      busIndexMap.clear();

      JsonArray predictions = doc["bustime-response"]["prd"];

      for (JsonObject prd : predictions) {
        const char* route = prd["rt"] | " ";
        const char* destination = prd["des"] | " ";
        const char* countdown = prd["prdctdn"] | " ";
        const char* direction = prd["rtdir"] | " ";

        std::string key = std::string(route) + "|" + direction;

        //RouteMap
        auto i = busIndexMap.find(key);
        if (i != busIndexMap.end()) {
          int value = i->second;
          Serial.print("Key Found for route: ");
          Serial.println(key.c_str());

          if(Routes[value].arrivalCount < MAX_ARRIVALS){
            Routes[value].arrivals[Routes[value].arrivalCount] = parseArrival(countdown);
            Routes[value].arrivalCount += 1;
          }else{
            Serial.print("Route is full, skipping arrival for: ");
            Serial.println(route);
          }


        } else {
          int slot = busIndexMap.size();
          if (slot < MAX_ROUTES) {
            busIndexMap.insert({ key, slot });
            Routes[slot].route = String(route);
            Routes[slot].destination = String(destination);
            Routes[slot].direction = String(direction);
            Routes[slot].arrivals[0] = parseArrival(countdown);
            Routes[slot].arrivalCount = 1;
          } else {
            Serial.println("Routes array full");
          }
        }
      }
    }
  } else {
    Serial.print("Error:");
    Serial.println(httpCode);

    
    Serial.println("Previous call data will be used if stored");
  }
}





void loop() {
  getArrivalTimes(RetrievedRoutes);

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
      if(group.arrivalCount == 0) continue;

      char line[40];
      int len = snprintf(line, sizeof(line), "%-3.3s %-16.16s %4.1s", 
        group.route.c_str(), 
        group.destination.c_str(), 
        group.direction.c_str());

      for(int i = 0; i < group.arrivalCount; i++){
        if(len >= ((int)sizeof(line))) break;
      
        const Arrival& a= group.arrivals[i];
        const char* sep = (i > 0) ? "," : "   ";

        if(a.isDue){
          len += snprintf(line + len, sizeof(line) - len, "%sDUE", sep);
        }else{
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
  delay(180000);
}