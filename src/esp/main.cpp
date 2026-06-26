#include <Arduino.h>
#include <SPI.h>

#include "actions.h"
#include "power.h"
#include "wifi/web.h"
#include "PostponedTask.h"

volatile bool espInterrupt = false;
volatile bool isBtnPress = false, isBtnPressTwice = false,
              waitingSecond = false;
volatile uint8_t last_state;
volatile uint32_t lastPress = 0;
volatile int clickCounter = 0;

void IRAM_ATTR testBtn() {
  uint16_t now = millis();

  if (digitalRead(D7)) {  // HIGH
    isBtnPress = true;

    clickCounter++;
    if (waitingSecond && (now - lastPress <= 700)) {
      waitingSecond = false;
      isBtnPressTwice = true;
      lastPress = now;
    } else {
      waitingSecond = true;
      lastPress = now;
    }
  }
}

void actionBtnManual() {
  if (isBtnPress && millis() - lastPress > 800) {
    if (isBtnPressTwice) {
      Serial.printf("isBtnPressTwice : %d\n", clickCounter);
    } else {
      Serial.printf("isBtnPress once : %d\n", clickCounter);
    }
    //sei();
    isBtnPress = false;
    isBtnPressTwice = false;
    waitingSecond = false;
   // sei();
  }
}

void setup(void) {  // todo ready set pin, load fast 250ms, init time after load
  SPI.end();
  Serial.begin(115200);
  display.init();

  delay(1500);
  Serial.println("\nstart");
 // pinMode(13, INPUT);
 // attachInterrupt(13, testBtn, CHANGE);

 // delay(50);
  setupLink();
  realTime.init();
  postponedTask.setTimeout(10, []() { setupPins(); });  // setup last
  postponedTask.setTimeout(16, []() { display.drawNow(); });  // setup last

  postponedTask.setInterval(15000, []() { realTime.updateComputedTime(); });
  postponedTask.setInterval(30000, [](uint8 id) { realTime.printTime(); });
  postponedTask.setInterval(1000, []() { pump.tick(); });

  if (!actionDueToTime()) addTaskManageSleep();

  //  Serial.printf("Esp load in: %lu\n", millis() - 7000);
  Serial.printf("\n\n\n %10s \n\n\n", "Next loop");

  //delay(500);
}

void loop(void) {
  if(isBtnPress) {
    isBtnPress = false;
  //  clickCounter++;
    Serial.printf("\n\ntestBtn: %d\n\n", clickCounter);
  }
  delay(50);
  postponedTask.tick();
}