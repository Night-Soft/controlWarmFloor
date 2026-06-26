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

CurrentTime CurrentTime::get(const RtcDateTime& time) {
  uint32 sumSeconds = time.Hour() * 3600 + time.Minute() * 60 + time.Second();
  return CurrentTime{.sumSeconds = sumSeconds,
                     .hour = time.Hour(),
                     .minutes = time.Minute(),
                     .seconds = time.Second()};
}

CurrentTime CurrentTime::get(uint32 seconds) {
  RtcDateTime time = RtcDateTime(seconds);
  return CurrentTime::get(time);
}

RealRtc::RealRtc() : rtc(Wire) { }

void RealRtc::init() {
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
  // delay(100);

  if (needUpdateTime()) {  
    setTimeFromNtp();
  }
  Serial.print("\nCurrent rtc time: ");
  printTime();
  updateComputedTime();
  if (!isRtcCorrect()) {  // todo show on display
    Serial.println("Rtc error!");
  }

}

bool RealRtc::isRtcCorrect() {
  // if (!Rtc.IsDateTimeValid()) 
  if (!rtc.GetIsRunning()) return false;
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  
  uint32 unix2050 = 2528305866;
  int64_t timeSinceCompile =
      rtc.GetDateTime().Unix32Time() - compiled.Unix32Time();
  return timeSinceCompile > 0 && timeSinceCompile < unix2050;
}

// check if a time update is needed if more than 24 hours have passed since the last update
bool RealRtc::needUpdateTime() {
  uint64 lastNtpTime = getLastSetNtpTime();
  uint64 curUnixRtc = rtc.GetDateTime().Unix64Time();
  uint64 timeFrom2000 = lastNtpTime - c_UnixEpoch32;
  if (timeFrom2000) {
    Serial.printf("\nlastNtpTime: %llu, curUnixRtc: %llu \n", lastNtpTime,
                  curUnixRtc);
    RtcDateTime last = RtcDateTime(timeFrom2000);
    printTime(&last);
  } else {
    Serial.printf("\nlastNtpTime < 0: %llu", timeFrom2000);
  }

  unsigned long unix2050 = 2528305866;
  unsigned long unixCompile = UNIX_TIMESTAMP;
  if (curUnixRtc > unix2050 || curUnixRtc < unixCompile) {
    Serial.println("Rtc has wrong time, cheak your wire!");
    Serial.print("Rtc wrong time: ");
    printTime();
    return true;
  }

  Serial.println("\nRtcTime: ");
  printTime();

  uint32 day = 3600 * 24 * 3; // 259200
  return (lastNtpTime + day < curUnixRtc);
}

uint64 RealRtc::getLastSetNtpTime() {
  uint64 lastNtpTime = 0;
  FileData data(&LittleFS, "/lastNtpTime.dat", 'A', &lastNtpTime, sizeof(lastNtpTime));
  data.read();
  return lastNtpTime;
}

void RealRtc::updateLastSetNtpTime(uint64 unix64time) {
  FileData data(&LittleFS, "/lastNtpTime.dat", 'A', &unix64time, sizeof(unix64time));
  data.updateNow();
  Serial.printf("\nUpdate memory /lastNtpTime.dat: %llu\n", unix64time);
}

bool RealRtc::setTimeFromCompile() {
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  rtc.SetDateTime(compiled);  // save in ds1307
  updateComputedTime();
  printTime();
  return true;
}

RtcDateTime RealRtc::getTime() { return rtc.GetDateTime(); }

CurrentTime RealRtc::getCurrentTime() {
  return CurrentTime::get(rtc.GetDateTime());
}

CurrentTime RealRtc::getComputedTime() {
  unsigned long timePassed = millis() - lastMillis;
  uint32 seconds = (timePassed + computedTime) / 1000;

  return CurrentTime::get(RtcDateTime(seconds));
}

void RealRtc::updateComputedTime() {
  //Serial.println("updateComputedTime from rtc");
  RtcDateTime currentRtc = rtc.GetDateTime();

  computedTime = currentRtc.Hour() * 3600 * 1000;
  computedTime += currentRtc.Minute() * 60 * 1000;
  computedTime += currentRtc.Second() * 1000;
  lastMillis = millis();
}

// print time as "D/M/Y | H:M:S", in current line
void RealRtc::printTime(uint32 seconds) {
  RtcDateTime dateTime = RtcDateTime(seconds);
  printTime(&dateTime);
}

// print time as "D/M/Y | H:M:S", in current line
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

  if ((*ntp).time == nullptr) {
        Serial.println("Error: setTimeFromNtp ntp->time = nullptr");
        if (this->ntp == nullptr) delete ntp;
        return false;
  }

  if(!ntp->time->forceUpdate()) {
        Serial.println("Error: while forceUpdate ntp time");
        if (this->ntp == nullptr) delete ntp;
        return false;
  }

  unsigned long ntpUnixTime = ntp->time->getEpochTime();
  unsigned long compileUnixTime = UNIX_TIMESTAMP;

  if (this->ntp == nullptr) delete ntp;


  bool isSuccess = false;
  if (ntpUnixTime > compileUnixTime) {
    Serial.println("SetTimeFromNtp: ");
    uint64 time = (uint64)ntpUnixTime;
    setTimeFromUnix(time);
    isSuccess = true;
  } else {
      RtcDateTime dataTime = RtcDateTime();
      dataTime.InitWithUnix32Time(ntpUnixTime);
      Serial.printf("\nError: setTimeFromNtp nNtpTime: %ld\n", ntpUnixTime);
      Serial.print("\nNtpTime is: ");
      printTime(&dataTime);
  }

  Serial.print("currentRtcTime: ");
  this->printTime();

  return isSuccess;
}

bool RealRtc::setTimeFromUnix(uint64& time) {
  RtcDateTime dataTime = RtcDateTime();
  dataTime.InitWithUnix32Time(time);
  rtc.SetDateTime(dataTime);  // save in ds1307
  updateComputedTime();
  updateLastSetNtpTime(time);

  Serial.print("\n SetTimeFromUnix: ");
  printTime(&dataTime);

  return true;
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

