#include "NtpTime.h"
#include "secret.h"

NtpTime::NtpTime() { init(); }

NtpTime::~NtpTime() {
    delete time;
    time = nullptr;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

bool NtpTime::init() {
  bool isConnected = setupWifi();
  if (isConnected == false) {
        Serial.println("NtpTime: No WiFi, call '.init later.'");
        return false;
  }

  time = new NTPClient(ntpUDP);
  time->begin();
  time->setTimeOffset(2 * 3600);  // UTC + 2, Починаючи з 29 березня 2026 буде UTC +3 

  return true;
}

bool NtpTime::setupWifi() {
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting");

  bool isConnected = (WiFi.status() == WL_CONNECTED);
  int current = millis();
  while (isConnected == false) {
    delay(300);
    isConnected = (WiFi.status() == WL_CONNECTED);
    Serial.print(".");
    if (millis() - current > 5000) {
      isConnected = false;
      break;
    }
  }

  this->isWiFiConnected = isConnected;
  if (isConnected) {
    Serial.printf("\n%s connected!\n", ssid);
  } else {
    Serial.printf("\nFailed connect to %s!", ssid);
  }

  return isConnected;
}

void NtpTime::print() { Serial.println(time->getFormattedTime()); }