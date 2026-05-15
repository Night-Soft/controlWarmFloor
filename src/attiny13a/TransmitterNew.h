#include <Arduino.h>

#define G433_SPEED 500
#include <Gyver433.h>

#include "ShiftR.h"
ShiftRegister shiftR;

#define TRANSMITTER_PIN PB3
Gyver433_TX<TRANSMITTER_PIN> transmitter;

const char on[] = "ONN";
const char off[] = "OFF";

void sendSateHeating(bool isHeating) {
  if (isHeating) {
    transmitter.sendData(on);  // onn
  } else {
    transmitter.sendData(off);  // onn
  }

  // byte backupPinStatus = shiftR.pinStatus;
  // shiftR.pinStatus = 0;

  // shiftR.blink(BLUE_Q_LED);

  // shiftR.pinStatus = backupPinStatus;
  // shiftR.write();
}
