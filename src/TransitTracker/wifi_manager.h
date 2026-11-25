#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

bool ensureWiFi(const char* ssid, const char* password, unsigned long timeoutMs);

#endif