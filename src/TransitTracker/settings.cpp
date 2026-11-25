#include "settings.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;

constexpr uint32_t DEFAULT_REFRESH_SEC = 180;
constexpr uint32_t MIN_REFRESH_SEC = 60;
constexpr uint32_t MAX_REFRESH_SEC = 900;

//Reset Button Global Vars
constexpr unsigned long RESET_HOLD_MS = 3000UL;

Settings loadSettings(){
  Settings s;
  //Open the Namespace/storage area
  bool opened = prefs.begin("tracker", true);
  if(!opened){
    Serial.println("No saved settings yet, using config.h defaults");
  }

  //default values to use if nothing is saved
  s.ssid = prefs.getString("ssid", ssid);
  s.password = prefs.getString("password", password);
  s.stopIds = prefs.getString("stops", stopid);
  s.refreshSec = prefs.getUInt("refresh", DEFAULT_REFRESH_SEC);
  s.busAPIKey = busAPIKey;

  //forced reset of refresh sec
  s.refreshSec = constrain(s.refreshSec, MIN_REFRESH_SEC, MAX_REFRESH_SEC);

  //closing namespace
  prefs.end();

  //print values
  Serial.print("Settings: ssid=");
  Serial.print(s.ssid);
  Serial.print(" stops=");
  Serial.print(s.stopIds);
  Serial.print(" refresh=");
  Serial.print(s.refreshSec);
  Serial.println("s");

  return s;

}

bool resetHeld() {
  if(digitalRead(BOOT_PIN) == HIGH) return false;

  unsigned long start = millis();

  while (digitalRead(BOOT_PIN) == LOW && millis() - start < RESET_HOLD_MS) {
    delay(10);
  }

  return digitalRead(BOOT_PIN) == LOW;
}


void factoryReset() {
  prefs.begin("tracker", false);
  prefs.clear();
  prefs.end();
  Serial.println("Settings has been reset. Release BOOT to restart");
  while (digitalRead(BOOT_PIN) == LOW) delay(10);
  ESP.restart();
}