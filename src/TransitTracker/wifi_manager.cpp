#include "wifi_manager.h"
#include <WiFi.h>

bool ensureWiFi(const char* ssid, const char* password, unsigned long timeoutMs){
  if (WiFi.status() == WL_CONNECTED) return true;
  if (strlen(ssid) == 0 || strlen(password) == 0) return false;

  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeoutMs) {
    delay(250);
    Serial.print(".");
    Serial.println();
  }

  return WiFi.status() == WL_CONNECTED;
}