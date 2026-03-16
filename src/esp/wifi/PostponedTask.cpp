#include "PostponedTask.h"

TASK_TYPE getTaskType(uint8 type) {
  TASK_TYPE t;
  switch (type) {
    case 1:
      t = TASK_TYPE::regural;
      break;
    case 2:
      t = TASK_TYPE::lambda;
      break;
    default:
      t = TASK_TYPE::empty;
      break;
  }
  return t;
}

Ids::Ids(uint8 maxIds){
  size_t sizePtr = sizeof(void*);
  size_t sizeId = sizeof(uint8);
  size_t sizeType = sizeof(uint8);
  size_t sizeOneEl = sizePtr + sizeId + sizeType;
  itemSize = sizeOneEl;
  ids = (uint8*)calloc(sizeOneEl, maxIds);
}

Ids::~Ids() {
  if (ids != nullptr) free(ids);
}

void Ids::add(CallbackId id, void* ptr, TASK_TYPE type) {
  if(has(id)) remove(id); // move callback to end of list

  Serial.printf("Add id: %d\n", id);

  //uint8 *ptrTask = (uint8*)&ptr;
  uint8 *ptrIds = ids + (size * itemSize);

  ptrIds[0] = id;
  ptrIds[1] = (uint8)type;
  memcpy(ptrIds + 2, &ptr, sizeof(ptr)); 
  size++;
}

void Ids::remove(CallbackId id) {
  uint8* ptr = ids;
  Serial.printf("Remove id: %d\n", id);

  for (uint8 i = 0; i < size; i++) {
    if (*ptr == id) {
      shiftMem(ptr);
      break;
    }
    ptr += itemSize;
  }
}

void Ids::remove(uint8* taskPtr) {
  uint8* ptrIds = ids;
  uint8*restored = nullptr; // ptr to task in tasks

  for (uint8 i = 0; i < size; i++) {
    memcpy(&restored, ptrIds + 2, sizeof(void*)); //create ptr to task in tasks from ids
    ptrIds += itemSize;
    if (taskPtr != restored) continue;

    shiftMem(ptrIds - itemSize);
    return;
  }
}

bool Ids::has(const CallbackId& id) {
  uint8 *ptr = ids;

  for(uint8 i = 0; i < size; i++) {
    if(*ptr == id) return true;
    ptr += itemSize;
  }

  return false;
}

TaskPT Ids::get(const CallbackId& id){
  uint8 *ptr = ids;
  TaskPT taskPt{.pointer = nullptr, .type = TASK_TYPE::empty};

  for(uint8 i = 0; i < size; i++) {
    if(*ptr == id) {
      memcpy(&taskPt.pointer, ptr + 2, sizeof(void*));
      taskPt.type = (TASK_TYPE)*(ptr + 1);
      return taskPt;
    }
    ptr += itemSize;
  }

  return taskPt;
}

//ptr[0] = id, ptr[1] = TASK_TYPE, ptr[2] = ptr  
TaskPT Ids::get(uint8* ptr) {
  uint8 *ptrIds = ids;
  TaskPT taskPt{.pointer = nullptr, .type = TASK_TYPE::empty};
  uint8* resotored = nullptr;

  for(uint8 i = 0; i < size; i++) {
    memcpy(&resotored, ptrIds + 2, sizeof(void*));
    ptrIds += itemSize;
    if(ptr != resotored) continue;

    taskPt.pointer = resotored;
    taskPt.type = (TASK_TYPE)(ptrIds - itemSize)[1];
    return taskPt;
    }

  return taskPt;
}

bool Ids::updateNumTasks(uint8 maxRegural, uint8 maxLambda) {
  uint8* reallocPtr = (uint8*)realloc(ids, itemSize * (maxRegural + maxLambda));
  if(reallocPtr == nullptr) return false;
  ids = reallocPtr;
  return true;
}

void Ids::shiftMem(uint8* dest) {
  uint8* source = dest + itemSize;
  uint8* endMemPtr = ids + (size * itemSize);
  size_t sizeCpy = endMemPtr - source;

  if (sizeCpy > 0) memcpy(dest, source, sizeCpy);  // dest, source, size
  size--;
}


Tasks::Tasks(uint8 maxRegural, uint8 maxLambda)
    : ids(maxRegural + maxLambda) {
  size_t sizeRegural = sizeof(Task) * maxRegural;
  size_t sizeLambda = sizeof(TaskLm) * maxLambda;
  maxSizeTasksInBytes = sizeRegural* maxRegural + sizeLambda * maxLambda; 
  beginTasks = (uint8*)calloc(sizeRegural + sizeLambda, sizeof(uint8));
  endTasks = beginTasks;
}

Tasks::~Tasks() { free(beginTasks); }

void Tasks::add(Task& task) {
  // if (isFreeMemForTask(sizeof(Task)) == false && needDefragmentaton()) {
  //   defragmentation();
  // }

  memcpy(endTasks, &task, sizeof(Task));  // dest, source, len
  ids.add(task.id, endTasks, TASK_TYPE::regural);

  sizeTasksInBytes += sizeof(Task);
  endTasks+= sizeof(Task);
  size++;
}

void Tasks::add(TaskLm& task) {

  memcpy(endTasks, &task, sizeof(TaskLm));  // dest, source, len
  ids.add(task.id, endTasks, TASK_TYPE::lambda);

  sizeTasksInBytes += sizeof(TaskLm);
  endTasks+= sizeof(TaskLm);
  size++;
}

void Tasks::remove(CallbackId id) {
  TaskPT taskPt = this->get(id);
  remove(taskPt);
}

void Tasks::remove(TaskPT& taskPt) {
  CallbackId id = 0;
  if (taskPt.type == TASK_TYPE::regural) {
    id = ((Task*)taskPt.pointer)->id;
    sizeTasksInBytes -= sizeof(Task);
  } else if (taskPt.type == TASK_TYPE::lambda) {
    id = ((TaskLm*)taskPt.pointer)->id;
    sizeTasksInBytes -= sizeof(TaskLm);
  } else {
    Serial.printf("Error remove taskPt, unknown type: %d\n", (int)taskPt.type);
    return;
  }

  ids.remove(id); // todo return bool  
}

void Tasks::removeList(TaskPT taskPt[], uint8 len) {
  for (uint8 i = 0; i < len; i++) {
    remove(taskPt[i]);
  }
}

bool Tasks::has(const CallbackId& id) { return this->has(id); }

bool Tasks::has(Callback callback) {
  TaskPT taskPT = this->get(callback);
  if (taskPT.type == TASK_TYPE::regural) return true;
  return false;
}

TaskPT Tasks::get(const CallbackId& id) { return ids.get(id); }

TaskPT Tasks::get(Callback callback) { // remove for , try in ids getCallback
  if (this->size == 0) return TaskPT();

  Task *task = nullptr;
  for (const TaskPT&taskPt : *this) {
    if(taskPt.type != TASK_TYPE::regural) continue;
    task = (Task*)taskPt.pointer;
    if(task->callback == callback) return taskPt;
  }

  return TaskPT();
}

//memcpy(buffer, ptr, sizeof(ptr)); копировать данные из объекта value
//memcpy(buffer, &ptr, sizeof(ptr)); копировать байты из памяти начиная с 0x, (копия value of ptr )
void Tasks::defragmentation() {
  Serial.println("Start defragmentation");

  uint8* curTask = nullptr;  // curTask is in tasks
  uint8* tasksPtr = beginTasks;
  uint8* ptrIds = ids.ids;
  uint8 shiftSize = 0;

  for(int i = 0; i < ids.size; i++) { 
    memcpy(&curTask, ptrIds + 2, sizeof(void*));

    TASK_TYPE type = getTaskType((uint8) * (ptrIds + 1));

    if(type == TASK_TYPE::regural) {
      shiftSize = sizeof(Task);
    } else if (type == TASK_TYPE::lambda) {
      shiftSize = sizeof(TaskLm);
    } else {
      Serial.println("Error defragmentation");
      return;
    }

    // copy the task to the beginning of memory
    memcpy(tasksPtr, curTask, shiftSize);

    // update pointer to task in ids
    memcpy(ptrIds + 2, &tasksPtr, sizeof(void*));

    ptrIds += ids.itemSize;
    tasksPtr += shiftSize;
  }

  endTasks = tasksPtr;
}

bool Tasks::needDefragmentaton(uint8 taskSize){
  return (!canAddTask(sizeof(TaskLm)) && isFreeMemForTask(sizeof(TaskLm)));
}

bool Tasks::canAddTask(uint8 taskSize) {
  return endTasks + taskSize < beginTasks + maxSizeTasksInBytes;
}

bool Tasks::isFreeMemForTask(uint8 taskSize) {
  return sizeTasksInBytes + taskSize <= this->maxSizeTasksInBytes;
}

bool Tasks::updateNumTasks(uint8 maxRegural, uint8 maxLambda) {
  if (ids.updateNumTasks(maxRegural, maxLambda) == false) return false;
  defragmentation();

  size_t sizeRegural = sizeof(Task) * maxRegural;
  size_t sizeLambda = sizeof(TaskLm) * maxLambda;
  maxSizeTasksInBytes = sizeRegural* maxRegural + sizeLambda * maxLambda; 
  uint8* newTasksPtr = (uint8*)realloc(beginTasks, sizeRegural + sizeLambda);

  if(newTasksPtr == nullptr) return false;
  if(newTasksPtr == beginTasks) return true;
  beginTasks = newTasksPtr;
  endTasks = beginTasks + sizeTasksInBytes;
  // updata tasks pointers in ids
  uint8* curTaskPtr = beginTasks;
  uint8* ptrIds = ids.ids;
  uint8 shiftSize = 0;
  for(uint8 i = 0; i < size; i++) { // todo check
    TASK_TYPE type = getTaskType((uint8)*(ptrIds + 1));

    if(type == TASK_TYPE::regural) {
      shiftSize = sizeof(Task);
    } else if (type == TASK_TYPE::lambda) {
      shiftSize = sizeof(TaskLm);
    } else {
      Serial.println("Error updateNumTasks");
      return false;
    }
    //copy pointer of cur task to ids
    memcpy(ptrIds + 2, &curTaskPtr, sizeof(void*));
    curTaskPtr += shiftSize;
    ptrIds += ids.itemSize;
  }

  return true;
}

TaskIterator Tasks::begin() {
  return TaskIterator(ids.ids, ids.itemSize, size);
}  // always return begin

TaskIterator Tasks::end() {
  return TaskIterator(ids.ids + (ids.itemSize * ids.size));
}  // always return end element

PostponedTask::PostponedTask()
    : tasks(MAX_REGURAL_TASK, MAX_LAMBDA_TASK), size(tasks.size) {
  taskToErase = (TaskPT*)calloc(MAX_REGURAL_TASK + MAX_LAMBDA_TASK, sizeof(TaskPT));
}

PostponedTask::PostponedTask(uint8 maxRegural, uint8 maxLambda)
    : tasks(maxRegural, maxLambda), size(tasks.size) {
  taskToErase = (TaskPT*)calloc(maxRegural + maxLambda, sizeof(TaskPT));
}

PostponedTask::~PostponedTask() {
  if(taskToErase != nullptr) free(taskToErase);
}

uint32 PostponedTask::add(uint32 timeMs, Callback callback, bool once) {
  if (tasks.needDefragmentaton(sizeof(Task))) {
    tasks.defragmentation();

    if (tasks.canAddTask(sizeof(Task)) == false) {
      Serial.println("Can`t add task, limit of memory!");
      return 0;
    }
  }

  if (this->has(callback)) {
    Serial.println("PostponedTask: callback already exsists.");
    return 0;
  };

  const CallbackId id = getTaskId(TASK_TYPE::regural); 
  Task task = {.once = once,
               .id = id,
               .delay = timeMs,
               .timer = (uint32)millis(),
               .callback = callback};

  Serial.printf("Add regural -> id: %d, delay: %d\n", task.id, task.delay);

  tasks.add(task);
  return (uint32)id;
}

uint32 PostponedTask::add(uint32 timeMs, CallbackLm callback, bool once) {
  if (tasks.needDefragmentaton(sizeof(TaskLm))) {
    tasks.defragmentation();

    if (tasks.canAddTask(sizeof(TaskLm)) == false) {
      Serial.println("Can`t add taskLm, limit of memory!");
      return 0;
    }
  }

  const CallbackId id = getTaskId(TASK_TYPE::lambda);
  TaskLm task = {.once = once,
                 .id = id,
                 .delay = timeMs,
                 .timer = (uint32)millis(),
                 .callback = callback};

  Serial.printf("Add lambda -> id: %d, delay: %d\n", task.id, task.delay);

  tasks.add(task);
  return id;
}

bool PostponedTask::remove(Callback callback) {
  TaskPT taskPT = tasks.get(callback);
  if(taskPT.pointer == nullptr) return false;
  tasks.remove(taskPT);
  return true;
}

bool PostponedTask::remove(CallbackId id) {
  tasks.remove(id);
  return true;
}

bool PostponedTask::has(Callback callback) { return tasks.has(callback); }

bool PostponedTask::has(CallbackId id) { return tasks.has(id); }

void PostponedTask::tick() { 
  if (this->size == 0) return;

  static uint32 timer = 0;
  uint32 now = millis();

  if(now - timer < this->minTickTime) return;
  timer = now;

  numToErase = 0; // todo remove inside

  //uint64 prev = micros64();

  for (const TaskPT& taskPt : this->tasks) {
    if (taskPt.type == TASK_TYPE::empty) {
      Serial.println("Task type error, empty");
      break;
    }

    if (taskPt.type == TASK_TYPE::regural) {
      //Serial.println("In TASK_TYPE::regural");

      regural = (Task*)taskPt.pointer;
      if (now - regural->timer >= regural->delay) {
        if (regural->once == true) taskToErase[numToErase++] = taskPt;
        regural->timer = now;
        regural->callback();
      }
      continue;
    }

    lambda = (TaskLm*)taskPt.pointer;
    if (now - lambda->timer >= lambda->delay) {
     // Serial.println("In TASK_TYPE::lambda");

      if (lambda->once == true) taskToErase[numToErase++] = taskPt;
      lambda->timer = now;
      lambda->callback(lambda->id);
    }
  }

  if (numToErase) {
    //Serial.println("try removeList");
    tasks.removeList(this->taskToErase, numToErase);
  }
  this->updateMinTickTime();

 // Serial.printf("BeetwenTick: %llu micros\n",(uint64)(micros64() - prev));

}

bool PostponedTask::updateNumTasks(uint8 maxRegural, uint8 maxLambda) { 
  if(!tasks.updateNumTasks(maxRegural, maxLambda)) return false;
  // todo improve log and realloc
  TaskPT* newPtr = (TaskPT*)realloc(taskToErase,(maxRegural + maxLambda) * sizeof(TaskPT));
  if(newPtr == nullptr) return false;
  return true;
}

void PostponedTask::removeList(Callbacks& list, uint8 len) {
  TaskPT taskPT;

  for (uint8 i = 0; i < len; i++) {
    taskPT = tasks.get(list[i]);
    if (taskPT.pointer == nullptr) continue;
    tasks.remove(taskPT);
  }
}

void PostponedTask::removeList(CallbacksIds &list, uint8 len) {
  for (uint8 i = 0; i < len; i++) {
    tasks.remove(list[i]);
  }
}

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
  uint32 minTime = 0xffffffff; //max

  Task *regural;
  TaskLm *lambda;
  for (const TaskPT& taskPt : this->tasks) {
    if(taskPt.type == TASK_TYPE::empty) {
      Serial.println("Task type error, empty");
      continue;
    }

    if(taskPt.type == TASK_TYPE::regural) {
      regural = (Task*)taskPt.pointer;
      setMinTickTime(regural->delay, regural->timer, now, minTime);
      continue;
    }
    lambda = (TaskLm*)taskPt.pointer;
    setMinTickTime(lambda->delay, lambda->timer, now, minTime);
  }

  //Serial.printf("minTickTime: %d, minTime: %d\n", minTickTime, minTime);
  minTickTime = minTime;
}

CallbackId& PostponedTask::getTaskId(TASK_TYPE type) {
  if (type == TASK_TYPE::regural) {
    if (taskId == 127) taskId = 26;
    return ++taskId;
  }
  if (type == TASK_TYPE::lambda) {
    if (taskLmId == 255) taskLmId = 154;
    return ++taskLmId;
  }
  return numToErase; // todo
}

PostponedTask postponedTask;