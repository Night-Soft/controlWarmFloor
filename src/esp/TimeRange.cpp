#include "TimeRange.h"
TimeRange::TimeRange(){}

TimeRange::~TimeRange() {
  if (json != nullptr) delete[] json;
  if (ranges != nullptr) delete[] ranges;
}

uint16 TimeRange::getWorkingTime(Range& r, uint8 hour, uint8 minutes) {
  uint16 now = hour * 60 + minutes;
  uint16 start = r.startHour * 60 + r.startMinute;
  uint16 end = r.durationHour * 60 + r.durationMinute + start;
  return (now >= start && now < end) ? end - now : 0;
}

// the time for pump on in minutes
uint16 TimeRange::getWorkingTime(uint8 hour, uint8 minutes) {
  if (isRead == false) read();

  uint16 result = 0;
  for (uint8 i = 0; i < size; i++) {
    result = getWorkingTime(ranges[i], hour, minutes);
    if (result > 0) return result;
  }
  return result;
}

// returns the amount of time remaining from the current moment until the next time-range
uint32 TimeRange::getTimeToNextRange(uint32 now) {
  uint32 nextStart = 0xffffffff;
  read();
  Serial.print("\nNearestTime: ");

  for (uint8 i = 0; i < size; i++) {
    uint32 start = ranges[i].startHour * 3600 + ranges[i].startMinute * 60;
    if (start < now) start += 24 * 3600;
    Serial.printf("%u, ", start - now);
    if (start - now > nextStart) continue;
    nextStart = start - now;
  }
  Serial.print("\n");

  return nextStart;
}

// todo from seconds
NextWakeUp TimeRange::getNextWakeUp(uint8 curHours, uint8 curMinutes) {
  if (isRead == false) read();

  NextWakeUp wakeUp = {0};
  for (uint8 i = 0; i < size; i++) {
    uint16 res = getWorkingTime(ranges[i], curHours, curMinutes);
    if (res == 0) continue;
    wakeUp.hour = ranges[i].startHour;
    wakeUp.minutes = ranges[i].startMinute;
    break;
  }

  return wakeUp;
}

void printTimeRanges(uint8* jsonData, uint8 len) {
  JsonDocument doc;
  deserializeJson(doc, jsonData, len);

  JsonArray arr = doc.as<JsonArray>();
  Serial.println("Print time ranges from json:"); 
  for (size_t i = 0; i < arr.size(); i++) {
    for (JsonVariant item : arr[i].as<JsonArray>()) {
      Serial.printf("%d, ", item.as<int>());
    }
    Serial.println();
  }
}

void printTimeRanges(Ranges& ranges, uint8 len) {
  Serial.println("Print time ranges from Range:");
  for (uint8 i = 0; i < len; i++) {
    Serial.printf("sH: %d, ", ranges[i].startHour);
    Serial.printf("sM: %d, ", ranges[i].startMinute);
    Serial.printf("dH: %d, ", ranges[i].durationHour);
    Serial.printf("dM: %d, ", ranges[i].durationMinute);
    Serial.println();
  }
}

FDstat_t TimeRange::write(uint8* jsonData, uint8 len) {
  FileData listTimeSize(&LittleFS, "/listTurnOnTimeSize.dat", 'c', &len, sizeof(size_t));
  FileData listTime(&LittleFS, "/listTurnOnTime.dat", 'B', jsonData, len);

  listTimeSize.updateNow();
  FDstat_t stat = listTime.updateNow();
  if (stat == FDstat_t::FD_FILE_ERR) {
    Serial.println("Create new files: listTurnOnTimeSize, listTurnOnTime.");
    listTimeSize.write();
    stat = listTime.write();
  }

  const char error[] = "Error";
  const char success[] = "Success";
  const char* statStr = stat == FDstat_t::FD_FILE_ERR ? error : success;

  Serial.print(statStr);
  Serial.println(" writing time ranges!");

  isUpdate = true;
  isRead = false;
  return stat;
}

void TimeRange::read() {
  if (isUpdate) {
    if (json != nullptr) delete[] json;
    if (ranges != nullptr) delete[] ranges;
    isUpdate = false;
  } else {
    if (json != nullptr && ranges != nullptr) return;
  }

  Serial.println("Raead time ranges.");
  
  size_t size = 0;
  FileData(&LittleFS, "/listTurnOnTimeSize.dat", 'c', &size, sizeof(size_t))
      .read();
  if (size == 0) {
    Serial.println("File 'listTurnOnTimeSize.dat' is empty!");
    return;
  }
  isRead = true;

  this->json = new char[size + 1]();
  FileData data(&LittleFS, "/listTurnOnTime.dat", 'B', this->json, size);
  this->status = data.read();

  JsonDocument doc;
  deserializeJson(doc, this->json, size);
  JsonArray arr = doc.as<JsonArray>();

  this->size = arr.size();
  this->ranges = (Ranges)new Range[arr.size()];
  uint8* rPtr = (uint8*)this->ranges;
  uint8 idx = 0;
  for (size_t i = 0; i < arr.size(); i++) {
    for (JsonVariant item : arr[i].as<JsonArray>()) {
      rPtr[idx++] = item.as<uint8>();
    }
  }
}

TimeRange timeRanges;