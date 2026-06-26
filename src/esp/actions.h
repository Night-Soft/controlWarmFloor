#ifndef ACTIONS_H
#define ACTIONS_H

#include "display.h"
#include "pins.h"
#include "pump.h"
#include "rtc/RealRtc.h"  // ds1307z
#include "linkAttiny.h"
#include "wifi/web.h"
#include "power.h"
#include "TimeRange.h"

enum class U_ACTIONS : uint8_t {
  pumpNoAction = 0,
  pumpTurnOn = 1,
  pumpTurnOnNSec = 2,
  pumpTurnOff = 3,
  webOn = 4,
  webOff = 5
};

struct NextAction {
  U_ACTIONS action;
  uint16_t operatingTime; // seconds
  NextAction()
    : action(U_ACTIONS::pumpNoAction), operatingTime(0) {}
};

uint16_t getSumSeconds(CurrentTime& time);

NextAction getBtnAction(STATE_BTN state);
NextAction getHeatingAction(STATE_HEATING state);

void onHeatingAndBtnMsg(MAIN_STATE &prev);
void onHeatingMsg(MAIN_STATE & prev);
void onBtnMsg(MAIN_STATE & prev);
void onTimeAction();
void executeAction(NextAction action);

bool actionDueToTime();
void checkActionByTime(uint8 time);
void stopActionByTime();

template <typename... Args>
void log(Args ...args) {
 // Serial.print(args,)
}

#endif