#include "linkAttiny.h"
#include "power.h"

// volatile bool tes = false;
// volatile uint32_t bits[100];
// volatile uint8 pStates[100];
// volatile short idxBits = 0;
// volatile uint32 timerBits = 0;
IRAM_ATTR void pinChange() {
  rx.pinChange();

  //tes = true;
  // unsigned long mc = micros();
  // if (idxBits > 0) {
  //   unsigned long beetwebMc = 0;
  //   beetwebMc = mc - bits[idxBits - 1];
  //   bits[idxBits++] = beetwebMc;
  // } else {
  // }
  //   pStates[idxBits] = digitalRead(P_WIRE_ATTINY);
  //   bits[idxBits++] = micros();

  // timerBits = millis();
}

const char* getTypeHeating(STATE_HEATING state) {
  switch (state) {
    case STATE_HEATING::OFF:
      return "OFF";
      break;
    case STATE_HEATING::ON:
      return "ON";
      break;
    case STATE_HEATING::UNDEFINED:
      return "UNDEFINED";
      break;
  }
  return "UNDEFINED_";
}

void setupLink() {
  attachInterrupt(P_WIRE_ATTINY, pinChange, CHANGE);

  auto onMessage = [](uint8_t type, void* data, size_t len) {
    Serial.print("\nMessage -> ");
    if (sizeof(MESSAGE) != len) return;  // проверка корректности длины
    stopActionByTime();

    MESSAGE& msg = *((MESSAGE*)data);

    switch (msg.type) {
      case MESSAGE_TYPE::FULL:
        Serial.print("type: FULL, ");
        Serial.printf(
            "state.btn: %u, state.heating: %s, state.prevHeating: %s\n\n",
            (uint8)msg.state.btn, getTypeHeating(msg.state.heating),
            getTypeHeating(msg.state.prevHeating));

        mainState = msg.state;
        onHeatingAndBtnMsg(mainState);
        break;
      case MESSAGE_TYPE::HEATING:
        Serial.printf(
            "type: HEATING, state.heating: %s, state.prevHeating: %s\n\n",
            getTypeHeating(msg.state.heating),
            getTypeHeating(msg.state.prevHeating));

        mainState.prevHeating = msg.state.prevHeating;
        mainState.heating = msg.state.heating;
        onHeatingMsg(mainState);
        break;
      case MESSAGE_TYPE::BTN_MANUAL:
        Serial.printf("type: BTN_MANUAL, state.btn: %u\n\n",
                      (uint8)msg.state.btn);

        mainState.btn = msg.state.btn;
        onBtnMsg(mainState);
        break;
    }
    if(msg.type != MESSAGE_TYPE::BTN_MANUAL) {
      onTimeAction();  // todo
    }
  };

  auto onRaw = [](void* data, size_t len) {
    Serial.println("onRaw");
    uint8* ptr = (uint8*)data;

    if ((ptr[0] >> 7) & 1) {
      Serial.println("onRaw invert");
      for (size_t i = 0; i < len; i++) {
        ptr[i] ^= 0xFF;
      }
    }
  };

  rx.onRaw(onRaw);
  rx.onPacket(onMessage);
  Serial.println("Add onMessage tick");
  postponedTask.setInterval(100, []() { rx.tick(); });
}

RX_t rx;