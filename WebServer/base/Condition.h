
#pragma once
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <cstdint>
#include "MutexLock.h"
#include "noncopyable.h"


class Condition : noncopyable {//用互斥锁完成条件变量
 public:
  explicit Condition(MutexLock &_mutex) : mutex(_mutex) {
    pthread_cond_init(&cond, NULL);
  }
  ~Condition() { pthread_cond_destroy(&cond); }
  void wait() { pthread_cond_wait(&cond, mutex.get()); }//这里的get时获取内部互斥锁变量
  void notify() { pthread_cond_signal(&cond); }
  void notifyAll() { pthread_cond_broadcast(&cond); }
  bool waitForSeconds(int seconds) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += static_cast<time_t>(seconds);
    return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);
  }

 private:
  MutexLock &mutex;
  pthread_cond_t cond;
};