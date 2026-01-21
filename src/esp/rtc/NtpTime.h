#ifndef NTP_TIME_H
#define NTP_TIME_H

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class NtpTime {
 private:
  WiFiUDP ntpUDP;
  bool setupWifi();

 public:
  NTPClient* time = nullptr; 
  bool isWiFiConnected = false;
  bool init();
  void print();
  NtpTime();
  ~NtpTime();
};

#endif