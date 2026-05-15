#ifndef SHIFT_R_H
#define SHIFT_R_H

#define DATA_PIN PB2    // к выводу 14 регистра
#define LATCH_PIN PB1  // к выводу 12 регистра (ST_CP)
#define CLOCK_PIN PB0 // к выводу 11 регистра (SH_CP)

#define Q0 0
#define Q1 1
#define Q2 2
#define Q3 3
#define Q4 4
#define Q5 5
#define Q6 6
#define Q7 7

#define RED_Q_LED 1
#define GREEN_Q_LED 2
#define BLUE_Q_LED 3

#include "AsArduino.h"

class ShiftRegister {
 public:
  ShiftRegister() { // uint8_t dataPin, uint8_t latchPin, uint8_t clockPin
    // this->dataPin = dataPin;
    // this->latchPin = latchPin;
    // this->clockPin = clockPin;
    
    pinMode(DATA_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);

    write(); // set all 0

  }

  void preparePin(uint8_t pin, uint8_t val, uint8_t BFIRST = 1) {
    if (BFIRST) {  // right
      if (val) {
        pinStatus |= SET_BIT_RIGHT(pin);  // high as 1
      } else {
        pinStatus &= CLEAR_BIT_RIGHT(pin);  // low as 0
      }
      return;
    }

    if (val) {
      pinStatus |= SET_BIT_LEFT(pin);  // high as 1
    } else {
      pinStatus &= CLEAR_BIT_LEFT(pin);  // low as 0
    }
  }

  void write(uint8_t pin, uint8_t val, uint8_t BFIRST = 1) {
    preparePin(pin, val, BFIRST);
    shift();

  }
  void write() { shift(); }

  void blink(uint8_t q_led) {
    for (uint8_t i = 0; i < 10; i++) {  // 1 seconds
      write(q_led, HIGH);
      delay(75);
      write(q_led, LOW);
      delay(25);
    }
  }
  byte pinStatus = 0;  // Начальное состояние

 private:
  void shift() {
    //digitalWrite(latchPin, LOW);                       // Отключаем защелку
    PORTB &= ~(1 << LATCH_PIN);
    shiftOut(DATA_PIN, CLOCK_PIN, LSBFIRST, pinStatus);  // Передаем данные

    PORTB |= (1 << LATCH_PIN);
    //digitalWrite(latchPin, HIGH);                      // Включаем защелку
  }

  // uint8_t dataPin;
  // uint8_t latchPin;
  // uint8_t clockPin;
};

extern ShiftRegister shiftR;

#endif
