#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "../wifi/wifi.h"

class NtpTime {
 private:
  WiFiUDP ntpUDP;
  WiFiHolder* wifiHolder = nullptr;

 public:
  NTPClient* time = nullptr; 
  bool isWiFiConnected = false;
  bool init();
  void print();
  NtpTime();
  ~NtpTime();
};

#endif