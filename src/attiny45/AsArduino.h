#ifndef AS_ARDUINO_H
#define AS_ARDUINO_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#ifndef HIGH
#define HIGH 1
#define LOW 0

#define OUTPUT 1
#define INPUT 0
#endif

#define delay(ms) (_delay_ms(ms))

#define enableInterrupt(PCINT) (PCMSK |= (1 << PCINT))
#define disableInterrupt(PCINT) (PCMSK &= ~(1 << PCINT))
#define allowExternalInterrupt() ( GIMSK |= (1 << PCIE))
#define forbidExternalInterrupt() ( GIMSK &= ~(1 << PCIE))


#define pinMode(pin, mode)                              \
  do {                                                   \
    if (mode)                                            \
      DDRB |= (1 << pin); /* Настроить pin как выход 1*/ \
    else                                                 \
      DDRB &= ~(1 << pin); /* Настроить pin как вход 0*/ \
  } while (0)

#define digitalRead(pin) ((PINB & (1 << pin)) ? 1 : 0)
#define digitalWrite(pin, value)                      \
  do {                                                 \
    if (value)                                         \
      PORTB |= (1 << pin); /* Установить бит (HIGH) */ \
    else                                               \
      PORTB &= ~(1 << pin); /* Сбросить бит (LOW) */   \
  } while (0)

#define SET_BIT_LEFT(pin) (1 << pin)
#define SET_BIT_RIGHT(pin) (1 << (7 - pin))
#define CLEAR_BIT_RIGHT(pin) (~(1 << (7 - pin)))
#define CLEAR_BIT_LEFT(pin) (~(1 << pin))

#define blink(pin, count, afterMs)     \
  for (uint8_t i = 0; i < count; i++) { \
    digitalWrite(pin, HIGH);           \
    delay(afterMs);                    \
    digitalWrite(pin, LOW);            \
    delay(afterMs);                    \
  }

#endif