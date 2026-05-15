#include "pins.h"

bool isMsgSended = false;
MESSAGE message = {.type = MESSAGE_TYPE::FULL,
                   .state = {STATE_HEATING::UNDEFINED, STATE_HEATING::UNDEFINED,
                             STATE_BTN::BTN_NOT_PRESSED}};

MAIN_STATE& mainState = message.state;

GwTx tx;

void sendState() {
  noInterrupts();
  tx.sendPacket(message);
  interrupts();
}

// void setFullState(STATE_HEATING stHeating, STATE_BTN stBtn, bool canSend) {
//   message.type = MESSAGE_TYPE::FULL;
//   mainState.prevHeating = mainState.heating;
//   mainState.heating = stHeating; 
//   mainState.btn = stBtn; 

//   if (canSend) allowSending();
// }

void sendFullState() {
  message.type = MESSAGE_TYPE::FULL;
  isMsgSended = false;
  allowSending();
}

void setHeatingState(STATE_HEATING state, bool canSend) { 
  if (isMsgSended == false && message.type != MESSAGE_TYPE::HEATING) {
    message.type = MESSAGE_TYPE::FULL;
  } 
  if(isMsgSended) {
    message.type = MESSAGE_TYPE::HEATING;
  }

  isMsgSended = false;
  if (state != mainState.heating) { 
      mainState.prevHeating = mainState.heating;
  }
  mainState.heating = state;

  if (canSend) allowSending();
}

void setBtnState(STATE_BTN state, bool canSend) {
  if (isMsgSended == false && message.type != MESSAGE_TYPE::BTN) {
    message.type = MESSAGE_TYPE::FULL;
  } 
  if(isMsgSended) {
    message.type = MESSAGE_TYPE::BTN;
  }

  isMsgSended = false;
  mainState.btn = state;

  if (canSend) allowSending();
}

bool needSend = false;
uint32_t sendStateTimer = 0;
void allowSending() {
 needSend = true;
 enableEsp();
 sendStateTimer = millis();
}

void sendStateIfNeeded() {
  uint32_t now = millis();
  if (needSend && espState == ESP_STATE::WORKING &&
      now - sendStateTimer > 100) {
    sendStateTimer = now;
    needSend = false;
    sendState();
    isMsgSended = true;

    if (message.type == MESSAGE_TYPE::FULL &&
        message.state.prevHeating == STATE_HEATING::ON &&
        message.state.heating == STATE_HEATING::OFF) {
      mainState.prevHeating = STATE_HEATING::UNDEFINED;
    }

      if (message.state.btn == STATE_BTN::BTN_PRESSED_TWICE) {
        message.state.btn = !digitalRead(P_BTN) ? STATE_BTN::BTN_HOLD
                                                : STATE_BTN::BTN_NOT_PRESSED;
      }
  }
}

void enableEsp() {  
  if (espState == ESP_STATE::WORKING) return;
  
  pinMode(P_ESP_RST, OUTPUT);

  digitalWrite(P_ESP_RST, LOW);
  delay(200);

  pinMode(P_ESP_RST, INPUT);
}

ESP_STATE getEspState() { return (ESP_STATE)digitalRead(P_ESP_STATE); }
