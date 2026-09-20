#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include "config.h"

//NetworkServer server(80);

const int rs = 12, en = 13, d4 = 14, d5 = 27, d6 = 26, d7 = 25;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(115200);
  // pinMode(5, OUTPUT);  // set the LED pin mode

  delay(1000);

  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
  // Print a message to the LCD.
  lcd.setCursor(0, 0); // Column 0, Row 0
  lcd.print("ESP32 Direct!");
  
  lcd.setCursor(0, 1); // Column 0, Row 1
  lcd.print("No I2C Adapter");

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void loop() {
  HTTPClient http;

  http.begin(String("https://www.ctabustracker.com/bustime/api/v3/getpredictions?key=") + busAPIKey + "&stpid=1593,18318&format=json"); //endpoint call

  int httpCode = http.GET();

  Serial.println(httpCode);

  if(httpCode > 0){
    Serial.print("Success:");
    Serial.println(httpCode);
    String payload = http.getString();

    //Json Deserialization
    DynamicJsonDocument doc(4096);  // size based on expected response — see note below
    DeserializationError error = deserializeJson(doc, payload);

    if(error){
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
      return;
    }

    JsonArray predictions = doc["bustime-response"]["prd"];

    Serial.println("************************");

    for(JsonObject prd : predictions){
      const char* route = prd["rt"];
      const char* dest = prd["des"];
      const char* dir = prd["rtdir"];
      const char* countdown = prd["prdctdn"];

      Serial.printf("Route %s to %s (%s): %s min\n", route, dest, dir, countdown);
    }

    Serial.println("************************");
  }else{
    Serial.print("Error:");
    Serial.println(httpCode);
  }

  http.end();

  delay(300000);

}
  