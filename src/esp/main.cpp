#include <Arduino.h>

#include "actions.h"

void setup(void) {
  Serial.begin(9600);
  delay(1500);
  Serial.println("start");
  realTime.init();
  triggerActionsByPins();
}

void loop(void) {
  delay(5000);
  Serial.println("\nBefore print time");
  realTime.printTime();
  pump.tick();
  draw();

}