#ifndef POWER_H
#define POWER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "pump.h"
#include "PostponedTask.h"
#include "display.h"
#include "actions.h"

extern Pump pump;

bool canSleep();
void resetManageCycleSleep();
void addTaskManageSleep();
void resetSleepTick();

#endif