#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#define LIGHT_INT_PIN PB4
//#include "AsArduino.h"

void setup_pin_change_interrupt() {
  DDRB &= ~(1 << LIGHT_INT_PIN);  // Настроить PB0 как вход
  PORTB |= (1 << LIGHT_INT_PIN);  // Включить Pull-up резистор на PB0

  GIMSK |= (1 << PCIE);  // Разрешаем Pin Change Interrupt
  PCMSK |= (1 << LIGHT_INT_PIN);   // Включаем прерывания для PB0 и PB2
  sei();             // Включаем глобальные прерывания
}

// void enter_sleep() {
//     // Настраиваем режим Power-down
//     set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//     sleep_enable();

//     // Входим в сон
//     sleep_cpu();

//     // После пробуждения
//     sleep_disable(); // Отключаем режим сна (опционально)
// }