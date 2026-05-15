#ifndef PINS_H
#define PINS_H

#include <Arduino.h>

using PINS = enum : uint8_t {
    P_SCA = 5,            // d1
    P_SDA = 4,            // d2
    P_ESP_STATE = 0,     // d3 - PB1
    P_WIRE_ATTINY = 14,   // d5 - PB2, 
    
    P_TOGGLE_PUMP = 12,    // d6 - bc547 in signal relay 
    P_TOGGLE_RELAY = 13,  // d7, gnd -  power relay
};

struct PINS_STATE {
    byte ESP_STATE;     // d5 - PB1
    byte TOGGLE_PUMP;   // d8, need test
};

void setupPins() ;
PINS_STATE getPinsState();

#endif
