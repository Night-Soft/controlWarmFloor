#include "PostponedTask.h"

PostponedTask::PostponedTask() {}
PostponedTask::~PostponedTask() {}

uint32 PostponedTask::add(uint32 timeMs, Callback callback, bool once) {
  Serial.println("Лямбда БЕЗ захвата");

  if (this->has(callback)) {
    Serial.println("PostponedTask: callback already exsists.");
    return (uint32)callback;
  };

  events.emplace(callback, Task{.once = once,
                                .delay = timeMs,
                                .timer = (uint32)millis(),
                                .callback = callback});

  this->ids.emplace_back((uint32)callback);
  this->updateSize();
  return (uint32)callback;
}

uint32 PostponedTask::add(uint32 timeMs, CallbackLm callback, bool once) {
    Serial.println("Лямбда с захватом");

    const uint32 id = this->taskId++;
    eventsLm.emplace(id, TaskLm{.once = once,
                                     .delay = timeMs,
                                     .timer = (uint32)millis(),
                                     .callback = callback});

    this->ids.emplace_back(id);
    this->updateSize();
    return id;
}

bool PostponedTask::has(Callback callback) {
  return events.find(callback) != events.end();
}

bool PostponedTask::has(uint32 id) {
  return eventsLm.find(id) != eventsLm.end();
}

bool PostponedTask::remove(Callback callback) {
  if (this->has(callback) == false) return false;
  events.erase(callback);
  this->updateSize();
  return true;
}

bool PostponedTask::remove(uint32 id) {
  if (this->has(id) == false) return false;
  eventsLm.erase(id);
  this->updateSize();
  return true;
}

void PostponedTask::removeList(Callbacks &list) {
  for (const auto& cb : list) {
    events.erase(cb);
  }
  this->updateSize();
}
void PostponedTask::removeList(CallbacksIds &list) {
  Callback ptr;
  for (const auto& id : list) {
    // erase id
    auto itIds = std::find(ids.begin(), ids.end(), id);
    if(itIds != ids.end()) ids.erase(itIds);
    // erase usual fn
    ptr = (Callback)(id);
    auto it = events.find(ptr);
    if(it != events.end()) {
      events.erase(ptr);
      continue;
    }
    // erase lamda
    auto itLm = eventsLm.find(id);
    if(itLm != eventsLm.end()) {
      eventsLm.erase(itLm);
    }
  }
  this->updateSize();
}

void PostponedTask::updateSize() {
  this->size = (uint8_t)(events.size() + eventsLm.size());
}

// template <typename F>
// void PostponedTask::forEach(F callback) {
//   bool needBreak = false;

//   for (const auto& pair : events) {
//     if (needBreak == true) break;

//     callback((Task&)pair.second, (Callback&)pair.second.callback, needBreak);
//   }
// }

void PostponedTask::setMinTickTime(const uint32& delay, const uint32& timer,
                                     const uint32& now, uint32& minTime) {
  if (delay < minTime) minTime = delay;

  int timeLeftNextCall = delay - (now - timer);  // (timePassed)
  if (timeLeftNextCall >= 0 && timeLeftNextCall < (int)minTime) {
    minTime = timeLeftNextCall;
  }
}

void PostponedTask::updateMinTickTime() {  // setMinTickTime
  if (this->size == 0) return;

  uint32 now = millis(); 
  uint32 minTime = 0xffffffff;

  for(const auto&[cb, task] : events) setMinTickTime(task.delay, task.timer, now, minTime);
  for(const auto&[id, task] : eventsLm) setMinTickTime(task.delay, task.timer, now, minTime);

  Serial.printf("minTickTime: %d, minTime: %d\n", minTickTime, minTime);
  minTickTime = minTime;
}

void PostponedTask::tick() { 
  if (this->size == 0) return;
  
  static uint32 timer = 0;
  uint32 now = millis();

  if(now - timer < this->minTickTime) return;
  timer = now;

  CallbacksIds idToErase;

  for (const auto& id : ids) {
    Callback ptr = (Callback)(id); 
    auto itEvents = events.find(ptr);
    if (itEvents != events.end()) {
      Task& task = itEvents->second;
      if (now - task.timer >= task.delay) {
        task.timer = now;
        task.callback();
        if (task.once == true) {
          idToErase.emplace_back(id);
        }
      }
      continue;
    }

    auto itEventsLm = eventsLm.find(id);
    if (itEventsLm != eventsLm.end()) {
      TaskLm& task = itEventsLm->second;
      if (now - task.timer >= task.delay) {
        task.timer = now;
        task.callback(id);
        if (task.once == true) {
          idToErase.emplace_back(id);
        }
      }
      continue;
    }
  }
  
  removeList(idToErase);
  this->updateMinTickTime();
}

PostponedTask postponedTask;