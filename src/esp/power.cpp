#include "power.h"

#include "TimeRange.h"

bool canSleep() {
  bool isAP = WiFi.getMode() == WIFI_AP;             // no sleep
  bool isConnected = WiFi.status() == WL_CONNECTED;  // no sleep
  bool isRelayOn = pump.isWorking();                 // no sleep
  return !(isAP || isConnected || isRelayOn);
}

void showSleepLeftTime(uint32* timeSec) {
  RtcDateTime afterTime = RtcDateTime(*timeSec);

  sprintf(display.modal.str2, "     after: %02u:%02u:%02u", afterTime.Hour(),
          afterTime.Minute(), afterTime.Second());

  Serial.println(display.modal.str1);
  Serial.println(display.modal.str2);
}

char sleepStr[40] = {0};
uint8 taskTickId = 0, taskSleepNowId = 0, modalId = 0, sleepCycleId = 0;
void addShowSleepLeftTime(uint32 timeSec = 0) {  // can run only postponed task
  resetManageCycleSleep();

  int count = 14;
  auto sleepTick = [count, timeSec](uint8 id) mutable {
    Serial.printf("sleepTick -> count: %d\n", count);

    if (!canSleep()) {
      resetManageCycleSleep();
      return;
    }

    timeSec--;
    showSleepLeftTime(&timeSec);

    if (count-- == 0) {
      display.powerSave();
      uint64_t us = timeSec * 1000 * 1000ull;
      Serial.printf("Sleep to: %llu\n", us);
      digitalWrite(P_ESP_STATE, HIGH);
      ESP.deepSleep(us); 
      Serial.println("This string should be not shown.");

      postponedTask.remove(id);
    }
  };

  Modal modal = {""};

  CurrentTime currentTime = realTime.getCurrentTime();
  RtcDateTime wakeUpTime = RtcDateTime(currentTime.sumSeconds + timeSec);
  RtcDateTime afterTime = RtcDateTime(timeSec);

  sprintf(modal.str1, "Wake up in: %02u:%02u:%02u", wakeUpTime.Hour(),
          wakeUpTime.Minute(), wakeUpTime.Second());
  sprintf(modal.str2, "     after: %02u:%02u:%02u", afterTime.Hour(),
          afterTime.Minute(), afterTime.Second());

  modalId = display.showModal(modal);

  postponedTask.clear(&sleepCycleId);
  taskTickId = postponedTask.setInterval(1000, sleepTick);
}

void resetManageCycleSleep() {
  if (taskTickId != 0) postponedTask.remove(taskTickId);
  if (taskSleepNowId != 0) postponedTask.remove(taskSleepNowId);
  if (modalId != 0 && display.getModalId() == modalId) display.hideModal();
  if (sleepCycleId == 0) addTaskManageSleep();
  taskTickId = 0;
  taskSleepNowId = 0;
  display.startUpdate();
}

void manageSleepCycle() {
  Serial.println("ManageSleepCycle");

  if (!canSleep()) return;

  if (actionDueToTime()) {
    postponedTask.clear(&sleepCycleId);
  } else {
    uint32 secondsForSleep =
        timeRanges.getTimeToNextRange(realTime.getComputedTime().sumSeconds);
    Serial.printf("Time for sleep: seconds: %u | ", secondsForSleep);
    realTime.printTime(secondsForSleep);
    addShowSleepLeftTime(secondsForSleep);
  }

  // if (secondsForSleep < 20 && mainState.heating == STATE_HEATING::ON) {
  //   postponedTask.clear(&sleepCycleId); // next add in executeAction ->
  //   resetManageCycleSleep checkActionByTime(secondsForSleep);
  // } else {
  //   addShowSleepLeftTime(secondsForSleep);
  // }
}

void resetSleepTick() {
  Serial.println("reset manageSleepCycle");

  resetManageCycleSleep();
  postponedTask.resetTick(sleepCycleId);
}

// sleep tick should be fast then deferred time
void addTaskManageSleep() {
  sleepCycleId = postponedTask.setInterval(10000, manageSleepCycle);
}