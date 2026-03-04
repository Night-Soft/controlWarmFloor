#include "PostponedTask.h"

//using Callback = std::function<void()>;
using Callbacks = std::vector<Callback>;
using Tasks = std::vector<Task>;

PostponedTask::PostponedTask() {}
PostponedTask::~PostponedTask() {}

//template <typename Cb>
void PostponedTask::add(uint32 timeMs, void(*callback)(), bool once) {
  if(this->has(callback)) {
    Serial.println("PostponedTask: callback already exsists.");
    return;
  };
  this->_size++;
  events.emplace(
      callback,
      Task{.once = once, .delay = timeMs, .timer = (uint32)millis(), .callback = callback});
}

bool PostponedTask::has(Callback callback) {
  return events.find(callback) != events.end();
}

bool PostponedTask::remove(Callback callback) {
  if (this->has(callback) == false) return false;
  events.erase(callback);
  this->_size--;
  return true;
}

size_t PostponedTask::size() { return (size_t)this->_size; }

template <typename F>
void PostponedTask::forEach(F callback) {
  bool needBreak = false;

  for (const auto& pair : events) {
    if (needBreak == true) break;

    callback((Task&)pair.second, (Callback&)pair.second.callback, needBreak);
  }
}

void PostponedTask::updateMinTickTime() {  // setMinTickTime
  if (this->_size == 0) return;

  uint32 now = millis(); 
  uint32 minTime = events.begin()->second.delay;

  this->forEach(
      [&minTime, &now](Task& task, Callback& cb, bool& needBreak) {
        if (task.delay < minTime) minTime = task.delay;

        int timeLeftNextCall = task.delay - (now - task.timer); // (timePassed)
        if (timeLeftNextCall >= 0 && timeLeftNextCall < (int)minTime) {
          minTime = timeLeftNextCall;
        }
      });
  //Serial.printf("minTickTime: %d, minTime: %d\n", minTickTime, minTime);
  minTickTime = minTime;
}

void PostponedTask::tick() {
  if (this->_size == 0) return;
  
  static uint32 timer = 0;
  uint32 now = millis();

  if(now - timer < this->minTickTime) return;
  timer = now;

  Callbacks cbToErase;

  this->forEach([&cbToErase, &now](Task& task, Callback& cb, bool& needBreak) {
    if (now - task.timer >= task.delay) {  // call all time passed callbacks // todo
      task.timer = now; // todo
      cb();

      if (task.once == true) {
        cbToErase.emplace_back(cb);
      }
    }
  });

  for(const auto& cb : cbToErase) {
    events.erase(cb);
    this->_size--;
  }

  this->updateMinTickTime();
}

PostponedTask postponedTask;

    // // Лямбда без захвата — конвертируется в указатель на функцию?
    // constexpr bool isConvertible = std::is_convertible_v<Cb, void(*)()>;

    // // Лямбда с захватом — НЕ конвертируется
    // constexpr bool hasCapture = std::is_class_v<Cb> && !isConvertible;

    // if constexpr (isConvertible && std::is_class_v<Cb>) {
    //     Serial.println("Лямбда БЕЗ захвата");
    // } else if constexpr (hasCapture) {
    //     Serial.println("Лямбда С захватом");
    // } else {
    //     Serial.println("Обычная функция");
    // }