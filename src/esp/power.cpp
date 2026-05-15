#include "power.h"
#include "TimeRange.h"

bool canSleep() {
  return pump.isWorking() == false && (WiFi.status() != WL_CONNECTED);
}

void showSleepLeftTime(uint32* timeSec) {
  RtcDateTime afterTime = RtcDateTime(*timeSec);

  sprintf(display.modal.str2, "     after: %02u:%02u:%02u", afterTime.Hour(),
          afterTime.Minute(), afterTime.Second());

  Serial.println(display.modal.str1);
  Serial.println(display.modal.str2);
}

char sleepStr[40] = {0};
uint8 taskTickId = 0, taskSleepNowId = 0, modalId = 0, deferrdeId = 0;
void addSleepTick(uint32 timeSec = 0) {  // can run only postponed task
  breakSleep();

  int count = 14;
  auto sleepTick = [count, timeSec](uint8 id) mutable {
    Serial.printf("sleepTick -> count: %d\n", count);

    if (!canSleep()) {
      breakSleep();
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

 // Modal modal = {"Deep sleep!"};
  Modal modal = {""};

  CurrentTime currentTime = realTime.getCurrentTime();
  RtcDateTime wakeUpTime = RtcDateTime(currentTime.sumSeconds + timeSec);
  RtcDateTime afterTime = RtcDateTime(timeSec);

  sprintf(modal.str1, "Wake up in: %02u:%02u:%02u", wakeUpTime.Hour(),
          wakeUpTime.Minute(), wakeUpTime.Second());
  sprintf(modal.str2, "     after: %02u:%02u:%02u", afterTime.Hour(),
          afterTime.Minute(), afterTime.Second());

  modalId = display.showModal(modal);

  postponedTask.clear(&deferrdeId);
  taskTickId = postponedTask.setInterval(1000, sleepTick);
}

// seconds to sleep
void trySleep(uint32 timeSec) {
  if (canSleep() == false) return;
  Serial.printf("Time for slep: %u\n", timeSec);
  addSleepTick(timeSec);
}

void breakSleep() {
  if (taskTickId != 0) postponedTask.remove(taskTickId);
  if (taskSleepNowId != 0) postponedTask.remove(taskSleepNowId);
  if(modalId != 0 && display.getModalId() == modalId) display.hideModal();
  if(deferrdeId == 0) addTaskCheckCanSleep();
  taskTickId = 0;
  taskSleepNowId = 0;
  display.startUpdate();
}

void deferredCheckCanSleep() {
  Serial.println("deferred Check Can Sleep");

  if(!canSleep()) return;

  uint32 secondsForSleep =
      timeRanges.getTimeToNextRange(realTime.getCurrentTime().sumSeconds);
  RtcDateTime timeSleep = RtcDateTime(secondsForSleep);
  Serial.printf("Time for sleep: seconds: %u | ", secondsForSleep);
  realTime.printTime(&timeSleep);

  if (secondsForSleep < 20 && mainState.heating == STATE_HEATING::ON) {
    postponedTask.clear(&deferrdeId); // next add in executeAction -> breakSleep
    checkActionByTime(secondsForSleep);
  } else {
    addSleepTick(secondsForSleep);    
  }
}

void resetSleepTick() {
  Serial.println("reset deferredCheckCanSleep");

  breakSleep();
  postponedTask.resetTick(deferrdeId);
}

// sleep tick should be fast then deferred time
void addTaskCheckCanSleep() { 
  deferrdeId = postponedTask.setInterval(10000, deferredCheckCanSleep);
}