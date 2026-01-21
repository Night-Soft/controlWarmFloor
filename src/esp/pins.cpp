#include "pins.h"

void initPins() {
    pinMode(P_BTN_MANUAL, INPUT_PULLDOWN_16);
    pinMode(P_STATE_HEATING, INPUT_PULLDOWN_16);
    pinMode(P_ESP_STATE, OUTPUT);
    pinMode(P_TOGGLE_PUMP, OUTPUT);

    digitalWrite(P_ESP_STATE, HIGH);
    digitalWrite(P_TOGGLE_PUMP, LOW); 
}

PINS_STATE getPinsState() {
  PINS_STATE state = {
      (byte)digitalRead(P_BTN_MANUAL),
      (byte)digitalRead(P_ESP_STATE),
      (byte)digitalRead(P_STATE_HEATING),
      (byte)digitalRead(P_TOGGLE_PUMP),
  };

  return state;
}