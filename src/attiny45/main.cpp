#include <Arduino.h>
#include <avr/sleep.h>

#define G433_SPEED 500
#include <Gyver433.h>

#include "pins.h"
#include "button.h"

#define RX_BUF_SIZE 4

Gyver433_RX<P_MX_RM, RX_BUF_SIZE> rx;

#define USED_PINS ((1 << P_BTN) | (1 << P_MX_RM) | (1 << P_ESP_STATE))

volatile bool espInterrupt = false;

volatile uint8_t lastPinsState = 0;

ISR(PCINT0_vect) {
  uint8_t current = PINB;
  uint8_t changed = (current ^ lastPinsState) & USED_PINS;

  if (changed & (1 << P_BTN)) {
    isBtnPress = true;
    canSleep = false;
  }

  if (changed & (1 << P_ESP_STATE)) {
    espInterrupt = true;
    canSleep = false;
    espState = (ESP_STATE)digitalRead(P_ESP_STATE);
  }

  if (changed & (1 << P_MX_RM)) rx.tickISR();  

  lastPinsState = current;
}

void setupInterrupt() {
  ADCSRA &= ~(1 << ADEN);  // ADC off
  ACSR |= (1 << ACD);      // компаратор off

  DDRB &= ~(1 << P_BTN);        // input
  //DDRB &= ~(1 << P_MX_RM);      // input
  DDRB &= ~(1 << P_ESP_STATE);  // input
  DDRB &= ~(1 << P_ESP_RST);    // input

  PORTB |= (1 << P_BTN);        // pull-up
  PORTB |= (1 << P_ESP_STATE);  // pull-up set resisor
  // PORTB |= (1 << P_MX_RM);  // pull-up set resisor

  // Включаем прерывание по изменению состояния пинов (PCINT)
  GIMSK |= (1 << PCIE);    // allow interrupt PCINT
  PCMSK |= (1 << PCINT1);  // allow only on PB1, P_ESP_STATE
  PCMSK |= (1 << PCINT3);  // allow only on PB3, P_BTN
 // PCMSK |= (1 << PCINT4);  // allow only on PB4, P_MX_RM

  sei();                   // allow interrupt
  // cli();                // forbid all interrupts
}

volatile ESP_STATE espState = ESP_STATE::WORKING; 
void onEspInterrupt() {
  static uint32_t timer = 0;
  uint32_t current = millis();
  if (timer == 0) timer = current;

  if (espInterrupt && current - timer > 100) {
    espInterrupt = false;
    timer = current;

    switch (espState) {
      case ESP_STATE::WORKING:
        sendFullState();
        allowSleep();
        break;
      case ESP_STATE::SLEEP:
        // if (mainState.heating == STATE_HEATING::OFF) {
        //   allowSleep();
        // } 
        allowSleep();
        break;
    }
  }
}

void onMxRm() {
  if (rx.gotData()) {
    const char on[] = "ONN";
    const char off[] = "OFF";

    char data[RX_BUF_SIZE] = {0};

    if (rx.readData(data)) {
      if (strcmp(data, on) == 0) {
        setHeatingState(STATE_HEATING::ON);
      } else if (strcmp(data, off) == 0) {
        setHeatingState(STATE_HEATING::OFF);
      }
      allowSleep();
    }
  }
}

volatile bool canSleep = false;
uint32_t sleepTimer = 0;
void allowSleep() {
    cli();
    canSleep = true;
    sei();
    sleepTimer = millis();
}

void sleepIfNeeded() {
  if(canSleep == false) return;

  uint32_t now = millis();
  if (canSleep && now - sleepTimer >= 5000) {
    //allowSleep();
    sleepTimer = now;
    sleep_cpu();
  }
}

void setup() {
  delay(100);

  allowSleep();
  setupInterrupt();
  espState = getEspState();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
  sleep_enable();
}

void loop() {
  delay(30);

  onBtnInterrupt();
  onEspInterrupt();
  onMxRm();

  sendStateIfNeeded();
  sleepIfNeeded();
}