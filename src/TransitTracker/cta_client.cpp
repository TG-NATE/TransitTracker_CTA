#include "cta_client.h" 

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <unordered_map>
#include <string>

static std::unordered_map<std::string, int> busIndexMap;


static Arrival parseArrival(const char* countdown) {
  Arrival a;

  a.isDue = (strcmp(countdown, "DUE") == 0);
  a.countdown = a.isDue ? 0 : atoi(countdown);
  return a;
}

bool getArrivalTimes(RouteGroup Routes[], const char* busAPIKey, const char* stopIds) {
  HTTPClient http;

  //Use Mozila root CA Bundle to verify CTA Bus Tracker API link. ESP32 core 4 refuses connection with out this.
  http.useBuiltinCACertBundle();
  http.begin(String("https://www.ctabustracker.com/bustime/api/v3/getpredictions?key=") + busAPIKey + "&stpid=" + stopIds + "&format=json");

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
      return false;

    } else {
      if (!doc["bustime-response"]["prd"].is<JsonArray>()) {
        Serial.println(doc["bustime-response"]["error"][0]["msg"] | " ");
        Serial.println("Previous call data will be used if stored");
        return false;
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

          if (Routes[value].arrivalCount < MAX_ARRIVALS) {
            Routes[value].arrivals[Routes[value].arrivalCount] = parseArrival(countdown);
            Routes[value].arrivalCount += 1;
          } else {
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

      return true;
    }
  } else {
    Serial.print("Error:");
    Serial.println(httpCode);


    Serial.println("Previous call data will be used if stored");
    return false;
  }
}

