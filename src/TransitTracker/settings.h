#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

struct Settings {
  String ssid;
  String password;
  String stopIds;
  String busAPIKey;
  uint32_t refreshSec;
};

Settings loadSettings();
bool resetHeld();
void factoryReset();

#endif