#ifndef TIME_RANGE_H
#define TIME_RANGE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FileData.h>
#include <LittleFS.h>  // Include the LittleFS library

//static constexpr uint8 JSON_COLS = 4;

struct Range {
  uint8_t startHour;
  uint8_t startMinute;
  uint8_t durationHour;
  uint8_t durationMinute;
};

struct NextWakeUp {
  uint8 hour, minutes;
};

using Ranges = Range*; 

void printTimeRanges(uint8* jsonData, uint8 len);
void printTimeRanges(Ranges& ranges, uint8 len);

class TimeRange {
  bool isUpdate = true; // after write show what is new time ranges but only in rom
  bool isRead = false;
 public:
  TimeRange();
  ~TimeRange();
  uint8 size = 0;
  Ranges ranges = nullptr;
  char * json = nullptr;
  FDstat_t status = FD_NO_DIF;

  uint16 getWorkingTime(Range& r, uint8 hour, uint8 minutes);
  // the time for pump on
  uint16 getWorkingTime(uint8 hour, uint8 minutes);
  // totalSeconds is current time (hours * 3600 + minutes * 60 + seconds), 
  // returns the amount of time( in seconds) remaining from the current moment until the next time-range
  uint32 getTimeToNextRange(const uint32 totalSeconds);
  NextWakeUp getNextWakeUp(uint8 curHours, uint8 curMinutes);
  void read();

  FDstat_t write(uint8* jsonData, uint8 len);
};

extern TimeRange timeRanges;
#endif