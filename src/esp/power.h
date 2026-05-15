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
//void addSleepTick();
void trySleep(uint32 timeSec = 0);
void breakSleep();
void addTaskCheckCanSleep();
void resetSleepTick();
#endif