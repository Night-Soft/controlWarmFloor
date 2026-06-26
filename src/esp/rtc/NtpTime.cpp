#include "NtpTime.h"

NtpTime::NtpTime() { init(); }

NtpTime::~NtpTime() {
    delete time;
    time = nullptr;
    if (this->wifiHolder != nullptr) {
      delete this->wifiHolder;
      this->wifiHolder = nullptr;
    }
}

bool NtpTime::init() {
  wifiHolder = new WiFiHolder();
  bool isConnected = wifiHolder->turnOn();
  if (isConnected == false) {
        Serial.println("NtpTime: No WiFi, call '.init later.'");
       // delete this->wifiHolder;
        return false;
  }
// todo auto offset
  time = new NTPClient(ntpUDP, 3 * 3600); // UTC + 2, Починаючи з 29 березня 2026 буде UTC +3
  time->begin();  
  return true;
}

void NtpTime::print() { Serial.println(time->getFormattedTime()); }