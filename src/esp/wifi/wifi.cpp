#include "wifi.h"
#include "../PostponedTask.h"
#include "../display.h"

int WiFiHolder::counter = 0;
void postTurnOffWifi();

WiFiHolder::WiFiHolder() {
  if(postponedTask.has(postTurnOffWifi)) {
    Serial.printf("Cancel turnOff wifi, remove task");
    postponedTask.remove(postTurnOffWifi);
  } 
  WiFiHolder::counter++;
  Serial.printf("Count of WiFiHolder is: %d\n", WiFiHolder::counter);
}

WiFiHolder::~WiFiHolder() {
  WiFiHolder::counter--;
  Serial.printf("Remove WiFiHolder: %d\n", WiFiHolder::counter);

  if(WiFiHolder::counter == 0 ) {
    Serial.println("~WiFiHolder: try add turnOffWiFi for 5000");
    postponedTask.setTimeout(5000, postTurnOffWifi);
  }

}

void WiFiHolder::turnOff() { turnOffWiFi(); }
bool WiFiHolder::turnOn() { return connectToWiFi(); }
bool WiFiHolder::isConnected() { return isWiFiConnected(); }

void postTurnOffWifi() {
      turnOffWiFi();
}

bool connectToWiFi() {
  if (isWiFiConnected()) return true;
   
  WiFi.begin(ssid, password);
  // Serial.printf("\nWifi status: %d", (int)(WiFi.status())); // 7
  Serial.print("\nConnecting");
  pinMode(LED_BUILTIN, OUTPUT);

  bool isConnected = (WiFi.status() == WL_CONNECTED);
  int current = millis();
  while (isConnected == false) {
    digitalWrite(LED_BUILTIN, HIGH);
    display.toggleWiFi(false);
    display.forceUpdade();
    delay(150);

    isConnected = (WiFi.status() == WL_CONNECTED);
    Serial.print(".");
    if (!isConnected && millis() - current > 15000) {
      isConnected = false;
      Serial.printf("\nWifi failed status: %d", (int)(WiFi.status()));
      turnOffWiFi();
      break;
    }
    digitalWrite(LED_BUILTIN, LOW); // low -> led is on
    display.toggleWiFi(true);
    display.forceUpdade();
    delay(150);
  }

  if (isConnected) {
    Serial.printf("\n%s connected!\n", ssid);
  } else {
    Serial.printf("\nFailed connect to %s!\n", ssid);
  }
  display.toggleWiFi(isConnected);
  return isConnected;
}

bool isWiFiConnected() { return WiFi.status() == WL_CONNECTED; }

void turnOffWiFi() {
  Serial.println("turnOffWiFi");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.forceSleepBegin();
  digitalWrite(LED_BUILTIN, HIGH); // turn off led
  display.toggleWiFi(false);
}