#include "actions.h"

RealRtc realTime;
Pump pump;
MAIN_STATE mainState = {.prevHeating = STATE_HEATING::UNDEFINED,
                        .heating = STATE_HEATING::OFF,
                        .btn = STATE_BTN::BTN_NOT_PRESSED};

uint16_t getSumSeconds(CurrentTime& time) {
  return time.hour * 3600 + time.minutes * 60 + time.seconds;
}

NextAction getBtnAction(STATE_BTN state) {
  U_ACTIONS action;
  bool isPumpWorking = pump.isWorking();

  if (state == STATE_BTN::BTN_PRESSED && isPumpWorking == 0) {
    action = U_ACTIONS::pumpTurnOn;
  }
  if (state == STATE_BTN::BTN_NOT_PRESSED && isPumpWorking == 1) {
    action = U_ACTIONS::pumpTurnOff;
  }
  if (state == STATE_BTN::BTN_NOT_PRESSED && isPumpWorking == 0) {
    action = U_ACTIONS::pumpNoAction;
    Serial.printf("U_ACTIONS::pumpNoAction, isPumpWorking: %d\n",
                  isPumpWorking);
  }
  if (state == STATE_BTN::BTN_PRESSED_TWICE) {
    action = isWebServer ? U_ACTIONS::webOff : U_ACTIONS::webOn;
  }

  NextAction next;
  next.action = action;
  return next;
}

NextAction getHeatingAction(STATE_HEATING state) { 
  CurrentTime current = realTime.getCurrentTime();
  NextAction heatingAction;

  if (state == STATE_HEATING::ON) {
    uint16 workingTime = timeRanges.getWorkingTime(current.hour, current.minutes);
    Serial.printf("Working time: %u\n", workingTime);
    if (workingTime) {
      heatingAction.action = U_ACTIONS::pumpTurnOnNSec;
      heatingAction.operatingTime = workingTime * 60;
    }
  }

  if (state == STATE_HEATING::OFF &&
      mainState.prevHeating == STATE_HEATING::ON) {
    heatingAction.action = U_ACTIONS::pumpTurnOnNSec;
    heatingAction.operatingTime = 15 * 60;  // 15 minutes after turn off
    Serial.printf("state == STATE_HEATING::OFF | %us\n", heatingAction.operatingTime);
  }

  switch (heatingAction.action) {
    case U_ACTIONS::pumpNoAction:
      Serial.println("getHeatingAction: pumpNoAction");
      break;
    case U_ACTIONS::pumpTurnOnNSec:
      Serial.println("getHeatingAction: pumpTurnOnNSec");
      Serial.printf("getHeatingAction: operatingTime: %u\n",
                    heatingAction.operatingTime);
      break;
    default:
      Serial.println("getHeatingAction: Error");
      break;
  }
  return heatingAction;
}

void onHeatingMsg(MAIN_STATE& prev) {
  Serial.println("onHeatingMsg");
  bool isHeating = mainState.heating == STATE_HEATING::ON;
  display.toggleHeating(isHeating);

  NextAction nextAction = getHeatingAction(mainState.heating);
  executeAction(nextAction);
}

void onBtnMsg(MAIN_STATE& prev) {
  NextAction nextAction = getBtnAction(mainState.btn);
  executeAction(nextAction);
}

void onHeatingAndBtnMsg(MAIN_STATE& prev) {
  NextAction btnAction = getBtnAction(mainState.btn);
  NextAction heitingAction = getHeatingAction(mainState.heating);
  bool isHeating = mainState.heating == STATE_HEATING::ON;
  display.toggleHeating(isHeating);

  NextAction nextAction;  // btnAction is high priority
  nextAction.action = btnAction.action;

  if (btnAction.action == U_ACTIONS::pumpNoAction) {
    nextAction.operatingTime = heitingAction.operatingTime;
    nextAction.action = heitingAction.action;
  }

  executeAction(nextAction);
}

uint8 actionByTimeId = 0, taskUpdateModalId = 0;
void checkActionByTime(uint8 time) {
  Serial.printf("add check ActionByTime: %u\n", time);

  Modal modal = {"", "Pump on after:", ""};
  sprintf(display.modal.str2, "%us", time);
  modal.timeShow = time;
  display.showModal(modal);

  taskUpdateModalId = postponedTask.setInterval(1000, [time](uint8 id) mutable {
    time--;  // 255
    sprintf(display.modal.str2, "%us", time);
    if (time > 0) return;

    Serial.println("clear taskUpdateModalId");
    postponedTask.clear(&id);
    //onHeatingMsg(mainState); // use this
    onTimeAction();
  });
}

bool actionDueToTime() {
  uint32 timeToNextRange =
      timeRanges.getTimeToNextRange(realTime.getComputedTime().sumSeconds);
  Serial.printf("Time to next range: %u seconds | ", timeToNextRange);
  realTime.printTime(timeToNextRange);

  if (timeToNextRange > 20) return false;

  uint8 time = timeToNextRange;
  Serial.println("actionDueToTime\n");

  Modal modal = {"", "Pump on after:", ""};
  sprintf(display.modal.str2, "%us", time);
  modal.timeShow = time;
  display.showModal(modal);
  realTime.getComputedTime();
  
  taskUpdateModalId = postponedTask.setInterval(1000, [time](uint8 id) mutable {
    time--;  // 255
    sprintf(display.modal.str2, "%us", time);
    if (time > 0) return;

    Serial.println("clear taskUpdateModalId");
    postponedTask.clear(&id);
    onTimeAction();
  });

  return true;
}

void stopActionByTime() {
  if (actionByTimeId == 0 && taskUpdateModalId == 0) return;

  Serial.println("stopActionByTime");
  postponedTask.clear(&actionByTimeId);
  postponedTask.clear(&taskUpdateModalId);
}

void onTimeAction() {
  Serial.println("onTimeAction");
  CurrentTime current = realTime.getCurrentTime();
  NextAction heatingAction;

  uint16 workingTime = timeRanges.getWorkingTime(current.hour, current.minutes);
  Serial.printf("Working time: %u\n", workingTime);
  if (workingTime) {
    heatingAction.action = U_ACTIONS::pumpTurnOnNSec;
    heatingAction.operatingTime = workingTime * 60;
  }
  executeAction(heatingAction);
}

void executeAction(NextAction heatingAction) {
  resetSleepTick();

  switch (heatingAction.action) { 
    case U_ACTIONS::pumpNoAction:
      Serial.println("No  action, can sleep");
      // todo check timeRanges and sleep if need
      break;
    case U_ACTIONS::pumpTurnOn: // btnmanual
      Serial.println("Action, 'pumpTurnOn'."); 
      pump.turnOnViaBtn();
      break;
    case U_ACTIONS::pumpTurnOff:
      Serial.println("Action, 'pumpTurnOff'.");
      pump.turnOff();
      break;
    case U_ACTIONS::pumpTurnOnNSec:
      Serial.printf("Action, pumpTurnOnNSec: %u\n", heatingAction.operatingTime);
      pump.turnOnAWhile(heatingAction.operatingTime);  // 15 minutes
      break;
    case U_ACTIONS::webOn:
      Serial.println("Enabling web server!");
      initWebServer();
      break;
    case U_ACTIONS::webOff:
      Serial.println("Turn of web server!");
      endServer();
      break;
  }
}
