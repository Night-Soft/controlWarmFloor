#ifndef POST_PONED_TASK_H
#define POST_PONED_TASK_H

#include <Arduino.h>
#include <functional>
#include <map>

using Callback = void(*)();

struct Task {
  bool once;
  uint32 delay;
  uint32 timer;
  Callback callback;
  // Callback calback;
};

using CbForEach = void(*)(Task&, Callback&, bool&); // value , key, needBreak


class PostponedTask {
 public:
  PostponedTask();
  ~PostponedTask();

//  template <typename Cb>
  void add(uint32 timeMs, Callback callback, bool once = true);

  void tick();

  bool has(Callback callback);
  bool remove(Callback callback);
  size_t size();

  template <typename F>
  void forEach(F Callback);

 private:
  uint8_t _size = 0;
  uint32 minTickTime = 1000;
  void updateMinTickTime();
  std::map<void(*)(), Task> events;
};

extern PostponedTask postponedTask;

#endif