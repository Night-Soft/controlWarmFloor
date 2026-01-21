#include <FileData.h>
#include <LittleFS.h>

#include "RealRtc.h"
#include "CompilationTime.h"

unsigned int getMaxTime(unsigned int a, unsigned int b, unsigned int c) {
  if (a > b && a > c) return a;
  if (b > a && b > c) return b;
  if (c > a && c > b) return c;
  return a;
}

RealRtc::RealRtc() : rtc(Wire) { }

void RealRtc::init() {  // todo init setup;
  LittleFS.begin();
  rtc.Begin(SDA, SCL); 

  if (!isRtcCorrect()) {
    Serial.println(
        "RTC is NOT running, let's set the time! Or replace battery.");
    Serial.println("RTC was not actively running, starting now");
    rtc.SetIsRunning(true);
  }

  // never assume the rtc was last configured by you, so
  // just clear them to your needed state
  rtc.SetSquareWavePin(DS1307SquareWaveOut_Low);
  delay(100);

  if (needUpdateTime()) {
    setTimeFromNtp();
  }
  Serial.print("\nCurrent time: ");
  printTime();
}

bool RealRtc::isRtcCorrect() {
  // if (!Rtc.IsDateTimeValid()) 
  if (!rtc.GetIsRunning()) return false;
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

  uint32_t timeSinceCompile =
      rtc.GetDateTime().Unix32Time() - compiled.Unix32Time();
  return timeSinceCompile > 0;
}

bool RealRtc::needUpdateTime() {
  uint32 lastNtpTime = getLastSetNtpTime();
  uint32 currentRtc = rtc.GetDateTime().Unix32Time();
  uint32 timeFrom2000 = lastNtpTime - c_NtpEpoch32FromUnixEpoch32;
  if (timeFrom2000) {
    Serial.printf("\nlastNtpTime: %u \n", lastNtpTime);
    RtcDateTime last = RtcDateTime(lastNtpTime);
    printTime(&last);
  } else {
    Serial.printf("\nlastNtpTime < 0: %u", timeFrom2000);
  }

  Serial.println("\ncurrent rtc");
  printTime();
  uint32 day = 3600 * 24;
  if (lastNtpTime + day > currentRtc) return false;

  updateLastSetNtpTime(currentRtc);
  return true;
}

uint32 RealRtc::getLastSetNtpTime() {
  uint32 lastNtpTime = 0;
  FileData data(&LittleFS, "/lastNtpTime.dat", 'A', &lastNtpTime, sizeof(lastNtpTime));
  data.read();
  return lastNtpTime;
}

void RealRtc::updateLastSetNtpTime(uint32 time) {
  FileData data(&LittleFS, "/lastNtpTime.dat", 'A', &time, sizeof(time));
  data.updateNow();
  Serial.printf("\nUpdate memory /lastNtpTime.dat %u", time);
}

bool RealRtc::setTimeFromCompile() {
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  rtc.SetDateTime(compiled);  // save in ds1307
  printTime();
  return true;
}

RtcDateTime RealRtc::getTime() { return rtc.GetDateTime(); }

CurrentTime RealRtc::getCurrentTime() {
  RtcDateTime currentRtc = rtc.GetDateTime();
  CurrentTime currentTime = {.hour = currentRtc.Hour(),
                             .minutes = currentRtc.Minute(),
                             .seconds = currentRtc.Second(),
                             .unixTime = currentRtc.Unix32Time()};
  return currentTime;
}

void RealRtc::printTime(RtcDateTime *dt) {
  RtcDateTime dataTime = rtc.GetDateTime();
  if (dt == nullptr) dt = &dataTime;

  char datestring[26];

  snprintf_P(datestring, countof(datestring),
             PSTR("%02u/%02u/%04u | %02u:%02u:%02u"), dt->Day(), dt->Month(), 
             dt->Year(), dt->Hour(), dt->Minute(), dt->Second());
  Serial.println(datestring);
}

bool RealRtc::setTimeFromNtp() {
  NtpTime* ntp = (this->ntp == nullptr ? new NtpTime() : this->ntp);

  unsigned long rtcUnixTime = rtc.GetDateTime().Unix32Time();
  unsigned long ntpUnixTime = ntp->time->getEpochTime();
  unsigned long compileUnixTime = UNIX_TIMESTAMP;
  unsigned long maxUnixTime =
      getMaxTime(rtcUnixTime, ntpUnixTime, compileUnixTime);

  RtcDateTime dataTime = RtcDateTime();
  dataTime.InitWithUnix32Time(maxUnixTime);

  rtc.SetDateTime(dataTime);  // save in ds1307

  Serial.print("\nNtpTime set: ");
  printTime();

  if (this->ntp == nullptr) delete ntp;

  if (ntpUnixTime < rtcUnixTime) {  // todo show error on display
    Serial.printf("\nError: setTimeFromNtp ntpUnixTime < rtcUnixTime: %ld < %ld",
                  ntpUnixTime, rtcUnixTime);
  }

  if (compileUnixTime > rtcUnixTime) {
    Serial.printf(
        "Error: setTimeFromNtp compileUnixTime > rtcUnixTime: %ld > %ld",
        ntpUnixTime, rtcUnixTime);
  }

  if(ntpUnixTime > rtcUnixTime && ntpUnixTime > compileUnixTime) {
    Serial.printf("\nSuccess setTimeFromNtp!");
  }

  return true; // todo
}

void RealRtc::initNtpTime() {
  if (ntp != nullptr) return;
  ntp = new NtpTime();
}

void RealRtc::deleteNtpTime() {
  if (ntp != nullptr) delete ntp;
  ntp = nullptr;
}

RealRtc::~RealRtc() { deleteNtpTime(); }

