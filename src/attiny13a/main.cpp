#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <Arduino.h>

#include "TransmitterNew.h"
#include "readLight.h"

#include "AsArduino.h"

byte flags = 0;
#define enableFlag(flagbit) (flags |= (1 << flagbit))
#define disableFlag(flagbit) (flags &= ~(1 << flagbit))
#define writeToFlag(flagbit, value)                 \
  do {                                              \
    if (value)                                      \
      (flags) |= (1U << (flagbit));                 \
    else                                            \
      (flags) &= ~(1U << (flagbit));                \
  } while (0)

#define flagState(flagBit) ((flags >> (flagBit)) & 1)
#define readHeating() (((PINB >> (LIGHT_INT_PIN)) & 1) ^1)// invert

#define millis16() ((uint16_t)millis())

//..F = flag
#define needSendF 0  // номер бита в flags
#define checkHeatingF 1
//#define needSleepF 2
#define checkHeatingLaterF 3
#define isInterruptF 4
#define isCallFromSetupF 5
#define isHeatingF 6

void setLed() {
  shiftR.pinStatus = 0;

  if (flagState(isHeatingF)) {  // todo
    shiftR.write(RED_Q_LED, HIGH);
  } else {
    shiftR.write(GREEN_Q_LED, HIGH);
  }
}

volatile static uint16_t timerLightInterrupt = 0;
ISR(PCINT0_vect) {
  timerLightInterrupt = millis16();
  enableFlag(isInterruptF);
  disableInterrupt(LIGHT_INT_PIN);
}

struct Timers {
    //uint16_t sleep = 0;
    uint16_t checkHeating = 0;
    uint16_t current = 0;
} timer;

void checkHeating(const uint16_t& currentTime) {
  if (flagState(checkHeatingF) && currentTime - timer.checkHeating >= 3000) {
    timer.checkHeating = currentTime;

    disableFlag(checkHeatingF);

    if (flagState(isCallFromSetupF)) {  // call once from setup
      if (flagState(isHeatingF)) enableFlag(needSendF); 
      
      disableFlag(isCallFromSetupF);
      return;
    }

    byte state = readHeating();
    if ((flagState(isHeatingF) && state) ||
        (!flagState(isHeatingF) && !state)) {
      enableFlag(needSendF);  // heating status will be sent 3 times
      return;
    }

    writeToFlag(isHeatingF, state);
    setLed();
  }
}

void needSend(const uint16_t& currentTime) {
  static uint8_t counter = 0;
  static uint16_t timerr = 0;
  if (flagState(needSendF) && millis16() - timerr >= 100) {
    timerr = currentTime;

    sendSateHeating(flagState(isHeatingF));

    counter += 1;
    if (counter == 3) {
      counter = 0;
      // timer.sleep = currentTime;
      disableFlag(needSendF);
      // shiftR.pinStatus = 0;
      // shiftR.write();
      byte backupPinStatus = shiftR.pinStatus;
      shiftR.pinStatus = 0;

      shiftR.blink(BLUE_Q_LED);

      shiftR.pinStatus = backupPinStatus;
      shiftR.write();

      sleep_cpu();
      // enableFlag(needSleepF);  // enter sleep after 3s
    }
  }
}

void testSend() {
static  bool b = false;

      delay(500);
     sendSateHeating(b);
     b = !b;
     byte backupPinStatus = shiftR.pinStatus;
     shiftR.pinStatus = 0;

     shiftR.blink(BLUE_Q_LED);

     shiftR.pinStatus = backupPinStatus;
     shiftR.write();

     timer.current = millis16();
}
int main(void) {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  setup_pin_change_interrupt();

  enableFlag(isCallFromSetupF);
  enableFlag(isInterruptF);
  bool b = false;
  for (;;) {
    testSend();
    if (b == true && flagState(isInterruptF) &&
        timer.current - timerLightInterrupt >= 500) {  // interrupt
      timerLightInterrupt = timer.current;

      writeToFlag(isHeatingF, readHeating());
      setLed();

      timer.checkHeating = timer.current;  // todo
      enableFlag(checkHeatingF);

      disableFlag(isInterruptF);
      enableInterrupt(LIGHT_INT_PIN);
    }

   // checkHeating(timer.current);  // always executed 3 times
   // needSend(timer.current);

    // if (flagState(needSleepF) && timer.current - timer.sleep >= 5000) {
    //   disableFlag(needSleepF);
    //   sleep_cpu();
    // }
  }
}