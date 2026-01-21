#ifndef ACTIONS_H
#define ACTIONS_H

#include "display.h"
#include "pins.h"
#include "pump.h"
#include "rtc/RealRtc.h"  // ds1307z

extern RealRtc realTime;
extern Pump pump;

enum PUMP_ACTION : uint8_t {
    pumpTurnOn = 1,
    pumpTurnOff = 2,
    pumpTurnOnNSec = 3,
    pumpNoAction = 4
};

enum BTN_MANUAL_ACTION : uint8_t {
  btnNoAction,
  btnIsPressed,
  btnIsPressedTwice
};

enum CHECK_FROM : uint8_t {fromInterrupt, fromSetup};

struct PUMP_ACTION_TIME {
  PUMP_ACTION action = pumpNoAction;
  uint16_t operatingTime = 0; // seconds
  PUMP_ACTION_TIME()
    : action(pumpNoAction), operatingTime(0) {}
};

void setupInterrupt();

IRAM_ATTR void changeStateHeating();
IRAM_ATTR void changeBtnManual();

uint16_t getSumSeconds(CurrentTime& time);
PUMP_ACTION getPumpActionByBtnManual(PINS_STATE& state);
PUMP_ACTION_TIME getPumpActionByTime(PINS_STATE& state, CHECK_FROM checkFrom);

void checkInerrupts();
void triggerActionsByPins();
void executeAction(PUMP_ACTION_TIME& pumpActionTime);
void checkCanSleep();

#endif