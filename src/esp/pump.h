#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

#include "pins.h"
#include "rtc/RealRtc.h"

class Pump {
  uint32 secondsAWhile =0;
  unsigned long endOfWorkTime = 0;  // millis
  bool isTick = false;

 public:
  bool isWorking();
  void turnOn();
  void turnOnAWhile(uint32 seconds);
  void turnOff();
  void tick();
  CurrentTime getEndTimeOfWork();
};

#endif