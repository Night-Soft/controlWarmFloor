#include "actions.h"

RealRtc realTime;
Pump pump;

CurrentTime turnOnTime = {.hour = 4, .minutes = 0, .seconds = 0};
CurrentTime turnOfTime = {.hour = 7, .minutes = 0, .seconds = 0};

void setupInterrupt() {
      attachInterrupt(digitalPinToInterrupt(P_STATE_HEATING), changeStateHeating, CHANGE);
      attachInterrupt(digitalPinToInterrupt(P_BTN_MANUAL), changeBtnManual, CHANGE);
}

volatile bool isStateHeatingInterupt = false;
IRAM_ATTR void changeStateHeating() { isStateHeatingInterupt = true; }

volatile BTN_MANUAL_ACTION btnAction = btnNoAction;
volatile uint32 lastBtnPressTime = 0; 
IRAM_ATTR void changeBtnManual() { 
  uint32 now = millis();

  uint32 timeBtwInterrupt = millis() - lastBtnPressTime;
  if(timeBtwInterrupt > 100 && timeBtwInterrupt < 200) {
    btnAction = btnIsPressedTwice;
  } else {
    lastBtnPressTime = now;
    btnAction = btnIsPressed;
  }

}

uint16_t getSumSeconds(CurrentTime &time) {
  return time.hour * 3600 + time.minutes * 60 + time.seconds;
}

PUMP_ACTION getPumpActionByBtnManual(PINS_STATE &state) {
  PUMP_ACTION action;
  if (state.BTN_MANUAL == 1 && state.TOGGLE_PUMP == 0) action = pumpTurnOn;
  if (state.BTN_MANUAL == 0 && state.TOGGLE_PUMP == 1) action = pumpTurnOff;
  if (state.BTN_MANUAL == 0 && state.TOGGLE_PUMP == 0) action = pumpNoAction;
  return action;
}

PUMP_ACTION_TIME getPumpActionByTime(PINS_STATE& state, CHECK_FROM checkFrom) { // todo check list turnOn
  CurrentTime current = realTime.getCurrentTime();
  short sumCurSeconds = getSumSeconds(current); 
  short sumTurnOnSeconds = getSumSeconds(turnOnTime);
  short sumTurnOffSeconds = getSumSeconds(turnOfTime);

  PUMP_ACTION_TIME pumpActionTime;

  if (state.STATE_HEATING == 1) {  // todo turnOn N times in day for 30 minutes
    if(sumCurSeconds >= sumTurnOnSeconds && sumCurSeconds < sumTurnOffSeconds ) {
      pumpActionTime.action = pumpTurnOnNSec;
      pumpActionTime.operatingTime = sumTurnOffSeconds - sumCurSeconds;
    }
  }

  if (state.STATE_HEATING == 0 && checkFrom == fromInterrupt) {
    pumpActionTime.action = pumpTurnOnNSec;
    pumpActionTime.operatingTime = 15 * 60; // 15 minutes after turn of
  }

  return pumpActionTime;
}

void checkInerrupts() { 
  static uint32 currentTime = 0;
  static uint32 timerCheckInterrupts = 0;
  currentTime = millis();

  if (currentTime - timerCheckInterrupts < 250) return;  
  timerCheckInterrupts = currentTime;

  PINS_STATE pinSt = getPinsState();
  PUMP_ACTION_TIME pumpActionTime;
  if (isStateHeatingInterupt) {
    isStateHeatingInterupt = false;
    pumpActionTime = getPumpActionByTime(pinSt, fromInterrupt);
  }

  // check btn interrupt, priority high for turnOn, turnOff
  if (currentTime - lastBtnPressTime > 250) {
    switch (btnAction) {
      case btnIsPressed:
        btnAction = btnNoAction;
        pumpActionTime.action = getPumpActionByBtnManual(pinSt);
        break;
      case btnIsPressedTwice:
        btnAction = btnNoAction;
        // create web server;
        return;
      case btnNoAction:
        Serial.println("btnNoAction");
        break;
    }
  }

  executeAction(pumpActionTime);
}

void triggerActionsByPins() {
    PINS_STATE pinSt = getPinsState();
    PUMP_ACTION_TIME pumpActionTime = getPumpActionByTime(pinSt, fromSetup);
    PUMP_ACTION btnPumpAction = getPumpActionByBtnManual(pinSt);

    if (btnPumpAction != pumpNoAction) pumpActionTime.action = btnPumpAction;
    executeAction(pumpActionTime);
}

void executeAction(PUMP_ACTION_TIME &pumpActionTime) {
    switch (pumpActionTime.action) {
    case pumpTurnOn:
      Serial.println("Action, 'pumpTurnOn'.");
      pump.turnOn();
      break;
    case pumpTurnOff:
      Serial.println("Action, 'pumpTurnOff'.");
      pump.turnOff();
      break;
    case pumpTurnOnNSec:
      Serial.println("Action, 'pumpTurnOnNSec'.");
      pump.turnOnAWhile(pumpActionTime.operatingTime);  // 15 minutes
      break;
    case pumpNoAction:
      Serial.println("No  action, can sleep");  // todo can sleep
      //checkCanSleep();
      break;
  }
}

void checkCanSleep() {
    static int8_t counter = 5;
    if(pump.isWorking() == false && realTime.ntp->isWiFiConnected == false) { // todo full compare
        static uint32 timer = 0;
        uint32 now = millis();
        if (now - timer > 1000) {
            timer = now;
            counter--;
            if (counter < 0) {
                Serial.println("Esp go to deep sleep.");
                ESP.deepSleep(0);
            }
        }

    } else if (counter != 5) counter = 5;
}
