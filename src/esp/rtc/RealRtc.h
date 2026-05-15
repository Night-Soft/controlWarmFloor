#ifndef REAL_RTC_H
#define REAL_RTC_H

#include <Arduino.h>
#include <RtcDS1307.h>
#include <Wire.h>  

#include "NtpTime.h"

struct CurrentTime {
  uint32 sumSeconds;
  uint8 hour, minutes, seconds;
  static CurrentTime get(const RtcDateTime &rtcdateTime);
  static CurrentTime get(uint32 seconds);
};

class RealRtc {
  private:
    bool needUpdateTime();
    uint64 getLastSetNtpTime();
    void updateLastSetNtpTime(uint64 time);
    
    unsigned long computedTime = 0;
    unsigned long lastMillis = 0;

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
  CurrentTime getComputedTime();
  void updateComputedTime();
  void printTime(RtcDateTime *dateTime = nullptr);

  bool setTimeFromNtp();
  void initNtpTime();
  void deleteNtpTime();
};

extern RealRtc realTime;
#endif