#ifndef PINS_H
#define PINS_H

#include <Arduino.h>
#include <GW_TX.h>
#include <string.h>

#include "AsArduino.h"

// VCC, PB2, PB1, PB0
// PB5, PB3, PB4, GND
enum PINS : uint8_t { 
  P_ESP_RST = PB0,    
  P_ESP_STATE = PB1,    // PB1 - d3, if HIGH esp in deep sleep, LOW working...
  P_WIRE_ESP = PB2,     // PB2 - d5, sends state btnManual, heating
  P_BTN = PB3,          // interrupt 
  P_MX_RM = PB4         // interrupt
};

enum class STATE_HEATING : uint8_t { OFF, ON, UNDEFINED };

enum class STATE_BTN : uint8_t {
  BTN_NOT_PRESSED,
  BTN_HOLD,  // the button is long pressed
  BTN_PRESSED_TWICE
};

enum class MESSAGE_TYPE : uint8_t { 
    BTN = 1, 
    HEATING = 2,
    FULL = 3
};

enum class ESP_STATE : uint8_t {
  SLEEP = 1,
  WORKING = 0
};

struct MAIN_STATE {
  STATE_HEATING prevHeating;
  STATE_HEATING heating;
  STATE_BTN btn;
};

struct MESSAGE {
  MESSAGE_TYPE type;
  MAIN_STATE state;
};

using GwTx = GW_TX<P_WIRE_ESP, 500>;

extern MAIN_STATE &mainState;
extern GwTx tx;
 
extern volatile ESP_STATE espState; 

void enableEsp();

extern volatile bool canSleep;
void allowSleep();

void setFullState(STATE_HEATING stHeating, STATE_BTN stBtn, bool canSend = true);
void sendFullState();
void allowSending();
void setHeatingState(STATE_HEATING state, bool allowSending = true);
void setBtnState(STATE_BTN state, bool allowSending = true);
void sendStateIfNeeded();

ESP_STATE getEspState();

#endif