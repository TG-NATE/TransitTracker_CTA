#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

//Eink Libs
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

#include "config.h"
#include "transit_types.h"

// --- DISPLAY HARDWARE CONSTRUCTOR MAP ---
// Maps to the pin layout: CS=5, DC=17, RST=16, BUSY=4
GxEPD2_BW<GxEPD2_420_GDEY042T81, GxEPD2_420_GDEY042T81::HEIGHT> display(
  GxEPD2_420_GDEY042T81(/*CS=*/ 5, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)
);




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

  display.hibernate(); // Puts the display into deep sleep to protect its elements

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


void bootUpText()
{
  char line[40];
  snprintf(line, sizeof(line), "Connecting to WIFI");


  display.setRotation(0); // Sets landscape orientation
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(line, 0, 0, &tbx, &tby, &tbw, &tbh);
  
  // Center text inside the bounding window boundaries
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(line);
  }
  while (display.nextPage()); // Executes the dynamic paging drawing loops

}


void getArrivalTimes(RouteTracker Routes){
  HTTPClient http;
  http.begin(String("https://www.ctabustracker.com/bustime/api/v3/getpredictions?key=") + busAPIKey + "&stpid=" + stopid +"&top=4&format=json");

  int httpCode = http.GET();
  Serial.println(httpCode);

  if (httpCode > 0) {
    Serial.print("Success:");
    Serial.println(httpCode);

    Serial.println("Downloading Payload");
    String payload = http.getString();

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("JSON parse failed: ");
      Serial.println(error.c_str());
    } else {
      JsonArray predictions = doc["bustime-response"]["prd"];
      
      for (JsonObject prd : predictions) {
          const char* route = prd["rt"];
          const char* dest = prd["des"];
          const char* countdown = prd["prdctdn"];

          char line[30];
          if (strcmp(countdown, "DUE") == 0) {
            snprintf(line, sizeof(line), "R%s %s DUE", route, dest);
          } else {
            snprintf(line, sizeof(line), "R%s %s %sm", route, dest, countdown);
        }

        Serial.println(line);

      }
    }
  } else {
    Serial.print("Error:");
    Serial.println(httpCode);
  }

  http.end();
}





void loop() {
  getArrivalTimes();
  Serial.println("Well HIYA!");

  // if (httpCode > 0) {
  //   Serial.print("Success:");
  //   Serial.println(httpCode);
  //   String payload = http.getString();

  //   DynamicJsonDocument doc(4096);
  //   DeserializationError error = deserializeJson(doc, payload);

  //   if (error) {
  //     Serial.print("JSON parse failed: ");
  //     Serial.println(error.c_str());
  //   } else {
  //     JsonArray predictions = doc["bustime-response"]["prd"];

  //     display.init(115200, false, 2, false);  // false: not the first boot, avoid extra flash
  //     display.setRotation(0);
  //     display.setFont(&FreeMonoBold9pt7b);
  //     display.setTextColor(GxEPD_BLACK);

  //     int16_t tbx, tby; uint16_t tbw, tbh;

  //     display.setFullWindow();
  //     display.firstPage();
  //     do {
  //       display.fillScreen(GxEPD_WHITE);
  //       uint16_t y = 15;

  //       for (JsonObject prd : predictions) {
  //         const char* route = prd["rt"];
  //         const char* dest = prd["des"];
  //         const char* countdown = prd["prdctdn"];

  //         char line[30];
  //         if (strcmp(countdown, "DUE") == 0) {
  //           snprintf(line, sizeof(line), "R%s %s DUE", route, dest);
  //         } else {
  //           snprintf(line, sizeof(line), "R%s %s %sm", route, dest, countdown);
  //         }

  //         display.getTextBounds(line, 0, 0, &tbx, &tby, &tbw, &tbh);
  //         uint16_t x = ((display.width() - tbw) / 2) - tbx;

  //         display.setCursor(x, y);
  //         display.print(line);
  //         y += 15;
  //       }
  //     } while (display.nextPage());

  //     display.hibernate();
  //   }
  // } else {
  //   Serial.print("Error:");
  //   Serial.println(httpCode);
  // }

  // http.end();
  // delay(300000);

  return;
}