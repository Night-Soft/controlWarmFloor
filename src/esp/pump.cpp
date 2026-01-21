#include "pump.h"

#include "math.h"
bool Pump::isWorking() { return (bool)digitalRead(P_TOGGLE_PUMP); }

void Pump::turnOn() {
  if (isWorking() == true) return;

  Serial.println("P_TOGGLE_PUMP: HIGH");
  digitalWrite(P_TOGGLE_PUMP, HIGH);
}

void Pump::turnOnAWhile(uint32 seconds) {
  if (seconds < 3) {
    Serial.println("Error turnOnAWhile");
    return;
  }
  this->isTick = true;
  this->endOfWorkTime = millis() + seconds * 1000;
  this->secondsAWhile = seconds;
  turnOn();
}

void Pump::turnOff() {
  if (this->isTick) this->isTick = false;
  if (isWorking() == false) return;

  Serial.println("P_TOGGLE_PUMP: LOW");
  digitalWrite(P_TOGGLE_PUMP, LOW);
}

void Pump::tick() {
  if (this->isTick == false) return;

  if (millis() > this->endOfWorkTime) {
    Serial.printf("The pump worked for %u minutes\n", this->secondsAWhile * 60);
    this->turnOff();
  }
}

CurrentTime Pump::getEndTimeOfWork() {
  if (isTick == false) return CurrentTime{0};

  float endTimeSeconds = (float)(this->endOfWorkTime - millis()) / 1000;

  short hour = (short)floor(endTimeSeconds / 3600);
  endTimeSeconds -= (float)hour * 3600;

  short minutes = (short)floor(endTimeSeconds / 60);
  endTimeSeconds -= (float)minutes * 60;

  short seconds = (short)endTimeSeconds;

  return CurrentTime{hour, minutes, seconds, 0};
}