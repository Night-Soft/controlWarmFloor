#ifndef POST_PONED_TASK_H
#define POST_PONED_TASK_H

#include <Arduino.h>

#include <functional>
#include <map>

#define MAX_REGURAL_TASK 32
#define MAX_LAMBDA_TASK 16

// Callback for usual function
using Callback = void (*)();
// Callback for lambda function
using CallbackLm = std::function<void(uint32)>;

using Callbacks = Callback*;
using CallbacksIds = uint8*;
using CallbackId = uint8;

struct Task {
  bool once;
  CallbackId id;
  uint32 delay;
  uint32 timer;
  Callback callback;
};

struct TaskLm {
  bool once;
  CallbackId id;
  uint32 delay;
  uint32 timer;
  CallbackLm callback;
};

using CbForEach = void (*)(Task&, Callback&, bool&);  // value , key, needBreak

enum class TASK_TYPE : uint8 { empty = 0, regural = 1, lambda = 2 };

TASK_TYPE getTaskType(uint8 type);

struct TaskPT { //pointer to Task and type
  void* pointer = nullptr;
  TASK_TYPE type = TASK_TYPE::empty;
};

class Ids {
 public:
  Ids(uint8 maxIds);
  ~Ids();

  void add(CallbackId id, void* ptr, TASK_TYPE type);
  void remove(CallbackId id);
  void remove(uint8* taskPtr);
  bool has(const CallbackId &id);
  TaskPT get(const CallbackId& id);
  TaskPT get(uint8 * ptr);
  bool updateNumTasks(uint8 maxRegural, uint8 maxLambda);
  CallbacksIds ids;
  uint8 size = 0; // num of items
  uint8 itemSize = 0; // size -> (id + type + ptr) in bytes 

 private:
  void shiftMem(uint8* ptr);
};

class TaskIterator {
 private:
  uint8* ptr = nullptr;
  uint8* end = nullptr;
  uint8 itemSize = 0;

  TaskPT taskPt{nullptr, TASK_TYPE::empty};  

 public:
  TaskIterator(uint8* ids) : ptr((uint8*)ids), end((uint8*)ids) {} // constructor for end() 
  TaskIterator(uint8* ids, uint8 itemSize, uint8 size) { // constructor for begin()
   // Serial.println("TaskIterator start");
    this->ptr = ids;  
    this->end = ids + itemSize * size;
    this->itemSize = itemSize;

    memcpy(&this->taskPt.pointer, ids + 2, sizeof(void*));
    this->taskPt.type = getTaskType(ids[1]); // ids[n] 0=id, 1=type, 2-6=ptr
  //  Serial.printf("TaskType: %d\n", (uint8)this->taskPt.type);

   // Serial.println("TaskIterator start end");
  }

  TaskPT& operator*() { return taskPt; }

  TaskIterator& operator++() { // todo nullpt
    //Serial.println("++");
    ptr+= itemSize;
    taskPt.type = getTaskType(ptr[1]);
    memcpy(&this->taskPt.pointer, this->ptr + 2, sizeof(void*)); // todo understand

    if (ptr > end) ptr = end;
    //Serial.printf("ptr: %u, end: %u\n", (uint32)ptr, (uint32)end);
    return *this;
  }

  bool operator!=(const TaskIterator& other) { return ptr != other.ptr; }
};

class Tasks {
 public: 
  Tasks(uint8 maxRegural, uint8 maxLambda);
  ~Tasks();

  void add(Task &task);
  void add(TaskLm &task);
  void remove(CallbackId id);
  void remove(TaskPT& taskPt);
  void removeList(TaskPT taskPt[], uint8 len);
  bool has(Callback callback);
  bool has(const CallbackId& id);
  TaskPT get(const CallbackId& id);    // return pointer to Task and type
  TaskPT get(const Callback callback); // return pointer to Task and type
  void defragmentation();
  bool needDefragmentaton(uint8 taskSize);
  bool isFreeMemForTask(uint8 taskSize);
  bool canAddTask(uint8 taskSize);
  bool updateNumTasks(uint8 maxRegural, uint8 maxLambda);

  TaskIterator begin();
  TaskIterator end();

  uint8* beginTasks = nullptr;
  uint8* endTasks = nullptr;
  uint8 size = 0; // num of items
  size_t sizeTasksInBytes = 0; // current all tasks size
  size_t maxSizeTasksInBytes = 0; // max available
  Ids ids;
};

class PostponedTask { 
 public:
  PostponedTask();
  PostponedTask(uint8 maxRegural, uint8 maxLambda);
  ~PostponedTask();

  uint32 add(uint32 timeMs, Callback callback, bool once = true);
  uint32 add(uint32 timeMs, CallbackLm callback, bool once = true);
  bool remove(CallbackId id);
  bool remove(Callback callback);
  bool has(CallbackId id);
  bool has(Callback callback);
  bool updateNumTasks(uint8 maxRegural, uint8 maxLambda);
  void tick();

  Tasks tasks;
  uint8 &size;

 private:
  void removeList(Callbacks& list, uint8 len);
  void removeList(CallbacksIds& list, uint8 len);
  void setMinTickTime(const uint32& delay, const uint32& timer,
                      const uint32& now, uint32& minTime);
  void updateMinTickTime();

  Task* regural;
  TaskLm* lambda;

  CallbackId& getTaskId(TASK_TYPE type);
  CallbackId taskId = 26;
  CallbackId taskLmId = 154;

  uint8 numToErase = 0;
  TaskPT* taskToErase = nullptr;

  uint32 minTickTime = 1000;
};

extern PostponedTask postponedTask;

#endif
