#ifndef LINK_ATTINY_H
#define LINK_ATTINY_H

#include <Arduino.h>
#include <GW_RX.h>

#include "pins.h"
#include "PostponedTask.h"

using RX_t = GW_RX<P_WIRE_ATTINY, 500>;
extern RX_t rx; 

enum class STATE_HEATING : uint8_t { OFF, ON, UNDEFINED };

enum class STATE_BTN : uint8_t {
  BTN_NOT_PRESSED,
  BTN_PRESSED,  // the button is long pressed
  BTN_PRESSED_TWICE
};

enum class MESSAGE_TYPE : uint8_t { 
    BTN_MANUAL = 1, 
    HEATING = 2,
    FULL = 3
};

enum class ESP_STATE : uint8_t {
  SLEEP = 0,
  WORKING = 1
};

enum class ACTION_TYPE : uint8_t {
    MSG_BTN_MANUAL = 1, 
    MSG_HEATING = 2,
    MSG_FULL = 3,
    SLEEP = 4
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

extern MAIN_STATE mainState;


void onHeatingAndBtnMsg(MAIN_STATE &state);
void onHeatingMsg(MAIN_STATE & state);
void onBtnMsg(MAIN_STATE & state);

void setupLink();

#endif