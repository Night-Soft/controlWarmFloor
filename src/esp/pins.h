#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

using PINS = enum : uint8_t {
    P_SCA = 5,            // d1
    P_SDA = 4,            // d2
    P_BTN_MANUAL = 13,    // d7 - PB3
    P_ESP_STATE = 14,     // d5 - PB1
    P_STATE_HEATING = 12, // d6 - PB2
    P_TOGGLE_PUMP = 15    // d8 // need test
};

struct PINS_STATE {
    byte BTN_MANUAL;    // d7 - PB3
    byte ESP_STATE;     // d5 - PB1
    byte STATE_HEATING; // d6 - PB2
    byte TOGGLE_PUMP;   // d8 // need test
};

void initPins() ;
PINS_STATE getPinsState();

#endif
