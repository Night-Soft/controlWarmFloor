#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

#include "pins.h"
#include "rtc/RealRtc.h"
#include "display.h"

enum class PUMP_STATE : uint8 {off, onAWhile, onViaBtn};

class Pump {
  uint32 secondsAWhile = 0;
  unsigned long endOfWorkTime = 0;  // millis
  unsigned long startTime = 0;  // millis
  bool canTick = false;
  PUMP_STATE state = PUMP_STATE::off;

 public:
  bool isWorking();
  void turnOnViaBtn();
  void turnOnAWhile(uint32 seconds);
  void turnOff();
  void tick();
  
  CurrentTime getEndTimeOfWork();
};

extern Pump pump;

#endif