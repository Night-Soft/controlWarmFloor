#include "pins.h"

void setupPins() {
  //  pinMode(P_WIRE_ATTINY, INPUT_PULLDOWN_16);
    pinMode(P_ESP_STATE, OUTPUT);
    pinMode(P_TOGGLE_RELAY, OUTPUT);
    pinMode(P_TOGGLE_PUMP, OUTPUT);

    digitalWrite(P_TOGGLE_PUMP, LOW);
    digitalWrite(P_TOGGLE_RELAY, LOW);
    digitalWrite(P_ESP_STATE, LOW);
}

PINS_STATE getPinsState() { 
  PINS_STATE state = {
      (byte)digitalRead(P_ESP_STATE),
      (byte)digitalRead(P_TOGGLE_PUMP),
  };

  return state;
}