#ifndef POST_PONED_TASK_H
#define POST_PONED_TASK_H

#include <Arduino.h>
#include <functional>
#include <map>

// Callback for usual function
using Callback = void(*)();
// Callback for lambda function
using CallbackLm = std::function<void(uint32)>;

using Callbacks = std::vector<Callback>;
using CallbacksIds = std::vector<uint32>;

struct Task {
  bool once;
  uint32 delay;
  uint32 timer;
  Callback callback;
};

struct TaskLm {
  bool once;
  uint32 delay;
  uint32 timer;
  CallbackLm callback;
};

using CbForEach = void(*)(Task&, Callback&, bool&); // value , key, needBreak


class PostponedTask {
 public:
  PostponedTask();
  ~PostponedTask();

  uint32 add(uint32 timeMs, Callback callback, bool once = true);
  uint32 add(uint32 timeMs, CallbackLm callback, bool once = true);

  void tick();

  bool has(Callback callback);
  bool has(uint32 id);
  bool remove(Callback callback);
  bool remove(uint32 id);
  uint8_t size;

  // template <typename F>
  // void forEach(F Callback);

 private:
  CallbacksIds ids;
  uint32 taskId = 1;
  uint32 minTickTime = 1000;
  void removeList(Callbacks &list);
  void removeList(CallbacksIds &list);
  void updateSize();
  void updateMinTickTime();
  void setMinTickTime(const uint32& delay, const uint32& timer,
                        const uint32& now, uint32& minTime);
  std::map<Callback, Task> events;
  std::map<uint32, TaskLm> eventsLm;
};

extern PostponedTask postponedTask;

#endif
