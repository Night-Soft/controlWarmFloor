#ifndef WI_FI_H
#define WI_FI_H

#include <ESP8266WiFi.h>

#include <array>

#include "secret.h"

bool connectToWiFi();
bool isWiFiConnected();
void turnOffWiFi();

class WiFiHolder {
 public:
  WiFiHolder();
  ~WiFiHolder();
  void turnOff();
  bool turnOn();
  bool isConnected();

  static int counter;
};

#endif