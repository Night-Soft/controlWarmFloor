#ifndef REAL_RTC_H
#define REAL_RTC_H

#include <Arduino.h>
#include <RtcDS1307.h>
#include <Wire.h>  

#include "NtpTime.h"

struct CurrentTime {
  short hour, minutes, seconds;
  uint32 unixTime;
};

class RealRtc {
  private:
    bool needUpdateTime();
    uint32 getLastSetNtpTime();
    void updateLastSetNtpTime(uint32 time);

 public:
  RealRtc();
  ~RealRtc();

  NtpTime* ntp = nullptr;
  RtcDS1307<TwoWire> rtc;

  void init();
  bool isRtcCorrect();
  bool setTimeFromCompile();
  RtcDateTime getTime();
  CurrentTime getCurrentTime();
  void printTime(RtcDateTime *dateTime = nullptr);

  bool setTimeFromNtp();
  void initNtpTime();
  void deleteNtpTime();
};

#endif