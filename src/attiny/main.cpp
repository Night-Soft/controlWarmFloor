#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#define G433_SPEED 500
#include <Gyver433.h>

#include "AsArduino.h"

enum PINS {
  P_ESP_RST = PB0,
  P_ESP_STATE = PB1,     // PB1 - d5, if LOW esp in deep sleep, HIGH working... 
  P_STATE_HEATING = PB2, // PB2 - d6
  P_BTN_MANUAL = PB3,    // PB3 - d7 // todo press twice
  P_MX_RM = PB4
};

#define RX_BUF_SIZE 2

Gyver433_RX<P_MX_RM, RX_BUF_SIZE> rx;
ISR(PCINT0_vect) { rx.tickISR(); }

void enableEsp() {
  if (digitalRead(P_ESP_STATE)) return;

  digitalWrite(P_ESP_RST, HIGH);
  delay(500);
  digitalWrite(P_ESP_RST, LOW);
}

int main() {
  pinMode(P_ESP_RST, OUTPUT);
  pinMode(P_STATE_HEATING, OUTPUT);

  pinMode(P_ESP_STATE, INPUT);
  pinMode(P_MX_RM, INPUT_PULLUP);

  digitalWrite(P_ESP_RST, LOW);  // todo
  digitalWrite(P_STATE_HEATING, LOW);

  // loop
  for (;;) {
    if (rx.gotData()) {
      char data[RX_BUF_SIZE];

      if (rx.readData(data)) {
        if (data[0] == 'O') {
          digitalWrite(P_STATE_HEATING, HIGH);
          enableEsp();
        } else if (data[0] == 'F') {
          digitalWrite(P_STATE_HEATING, LOW);
        }
      }
    }
  }
}