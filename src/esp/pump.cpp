#include "pump.h"
#include "math.h"

static uint8 maxStrLen = 9;
void clearLeftTime() {
  //Serial.println("clearLeftTime");

  // clear prev time
  FONT_SIZE prevFontSize = display.fontSize;
  display.setFontSize(FONT_SIZE::F17);
  uint8 witdh = display.gDisplay.getMaxCharWidth() * maxStrLen;
  uint8 height = display.gDisplay.getMaxCharHeight();

  display.gDisplay.setColorIndex(0);
  uint8 x, y;
  x = (128 - (witdh + 24)) / 2;  
  y = (64 - (height )) / 2; 
  display.gDisplay.drawBox(x, y, witdh, height);
  display.setFontSize(prevFontSize);

}

void printLeftTime(CurrentTime &time) {
  static char dateStr[12];

  if (time.hour) {
  sprintf(dateStr, "%02u:%02u:%02u", (uint8)time.hour, (uint8)time.minutes, (uint8)time.seconds);
  } else if (time.minutes) {
  sprintf(dateStr, "%02u:%02u", (uint8)time.minutes, (uint8)time.seconds);
  } else {
  sprintf(dateStr, "%02u", (uint8)time.seconds);
  }

  uint8 len = 0;
  char *dPtr = dateStr;
  while(dPtr[len] != '\0') len++;


  clearLeftTime();
 // Serial.printf("Left time: %s\n", dateStr);
  // draw
  display.setFontSize(FONT_SIZE::F17);
  uint8 witdh = display.gDisplay.getMaxCharWidth() * len;
  uint8 height = display.gDisplay.getMaxCharHeight();
  uint8 x, y;
  x = (128 - (witdh + 24)) / 2;  
  y = (64 - (height )) / 2;  
  display.gDisplay.setColorIndex(1);
  display.print(dateStr, x, y + 2);

  display.togglePump(true);
}

void togglePump(bool enable) {
  if(enable) {
    digitalWrite(P_TOGGLE_RELAY, HIGH);
    delay(500);
    digitalWrite(P_TOGGLE_PUMP, HIGH);
  } else {
    digitalWrite(P_TOGGLE_PUMP, LOW);
    delay(500);
    digitalWrite(P_TOGGLE_RELAY, LOW);
  }
}

bool Pump::isWorking() { return (bool)(digitalRead(P_TOGGLE_PUMP)); }

void Pump::turnOnViaBtn() {
  //if (isWorking() == true) return;

  Serial.println("turnOnViaBtn: HIGH");
 // digitalWrite(P_TOGGLE_PUMP, HIGH);
  togglePump(true);

  canTick = true;
  state = PUMP_STATE::onViaBtn;
  startTime = millis();
  display.togglePump(true);
  display.togglePumpBtn(true);
}

void Pump::turnOnAWhile(uint32 seconds) {
  //if (seconds < 3|| isWorking() == true) {
  if (seconds < 3) {
    Serial.printf("Error turnOnAWhile, seconds: %u\n", seconds);
    return;
  }
  this->canTick = true;
  this->endOfWorkTime = millis() + seconds * 1000;
  this->secondsAWhile = seconds;

  Serial.println("turnOnAWhile: HIGH");
  //digitalWrite(P_TOGGLE_PUMP, HIGH);
  togglePump(true);

  state = PUMP_STATE::onAWhile;
  display.togglePump(true);
}

void Pump::turnOff() {
  if (isWorking() == false) return;

  Serial.println("P_TOGGLE_PUMP: LOW");
  //digitalWrite(P_TOGGLE_PUMP, LOW);
  togglePump(false);

  canTick = false;
  state = PUMP_STATE::off;
  
  display.togglePump(false);
  display.togglePumpBtn(false);
  clearLeftTime();
}

void Pump::tick() {
  if (this->canTick == false) return;

  if (state == PUMP_STATE::onAWhile) {
    CurrentTime time = getEndTimeOfWork();
    printLeftTime(time);

    if (millis() > this->endOfWorkTime) {
      Serial.printf("The pump worked for %u minutes\n",
                    this->secondsAWhile / 60);
      Serial.printf("The pump isWorking: %d\n", isWorking());

      this->canTick = false;
      this->turnOff();
    }

  } else if (state == PUMP_STATE::onViaBtn) {
    CurrentTime time = CurrentTime::get((millis() - startTime) / 1000);

    printLeftTime(time);
  }
}

 // todo current time struct

CurrentTime Pump::getEndTimeOfWork() {
  if (canTick == false || this->endOfWorkTime <= millis()) return CurrentTime{0};

  uint32 endTimeSeconds = (this->endOfWorkTime - millis()) / 1000;

  return CurrentTime::get(endTimeSeconds);
}