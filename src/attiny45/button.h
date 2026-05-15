#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

#include "pins.h"

#define DEBOUNCE_MS 100
#define DOUBLE_MS 500
#define DELAYED_CHECK_MS 500

uint32_t lastDebounceTime = 0;
uint32_t lastClickTime = 0;

bool lastBtnState = 1; // BTN_NOT_PRESSED
bool waitingSecondClick = false;
bool needBtnCheck = false;
volatile bool isBtnPress = false;

bool pressed = false;

void setupButton() { lastBtnState = digitalRead(P_BTN); }

void onBtnInterrupt() {
  static STATE_BTN stateBtn = STATE_BTN::BTN_NOT_PRESSED;
  uint32_t now = millis();

  cli();
  if (isBtnPress) {
    isBtnPress = false;
    pressed = true;
  } 
  sei(); 

  if (pressed) {
    if (now - lastDebounceTime < DEBOUNCE_MS) return; 

    pressed = false;
    lastDebounceTime = now;

    bool rawState = digitalRead(P_BTN);
    //needBtnCheck = true; 

    if (rawState != lastBtnState) {
      needBtnCheck = true;
      lastBtnState = rawState;

      if (!rawState) { // LOW
        if (waitingSecondClick && (now - lastClickTime < DOUBLE_MS)) {
          waitingSecondClick = false;
          stateBtn = STATE_BTN::BTN_PRESSED_TWICE;
        } else {
          waitingSecondClick = true;
          lastClickTime = now;
        }
      }
    }
  }

  if (needBtnCheck && (now - lastClickTime > DELAYED_CHECK_MS)) {
    needBtnCheck = false;
    waitingSecondClick = false;

    if (stateBtn != STATE_BTN::BTN_PRESSED_TWICE) {
      stateBtn = !digitalRead(P_BTN)
                   ? STATE_BTN::BTN_HOLD
                   : STATE_BTN::BTN_NOT_PRESSED; 
    }

    setBtnState(stateBtn);

    stateBtn = STATE_BTN::BTN_NOT_PRESSED;
    allowSleep();
    // digitalWrite(P_ESP_RST, HIGH);
    // delay(100);
    // digitalWrite(P_ESP_RST, LOW);
  }
}

// void onBtnInterrupt() {
//   static STATE_BTN stateBtn = STATE_BTN::BTN_NOT_PRESSED;
//   static unsigned long timer = 0;
//   unsigned long now = millis();

//   if (isBtnPress) {
//     cli();
//     isBtnPress = false;
//     stateBtn = !digitalRead(P_BTN) ? STATE_BTN::BTN_HOLD : STATE_BTN::BTN_NOT_PRESSED;
//     setBtnState(stateBtn);
//     sendState(MESSAGE_TYPE::BTN);

//     digitalWrite(P_ESP_RST, HIGH);
//     delay(500);
//     digitalWrite(P_ESP_RST, LOW);
//     sei();
//   }
// }

#endif