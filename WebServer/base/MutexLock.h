
#pragma once
#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"
#include <semaphore.h>
#include <exception>

class MutexLock : noncopyable {//封装mutex
 public:
  MutexLock() { pthread_mutex_init(&mutex, NULL); }
  ~MutexLock() {
    pthread_mutex_lock(&mutex);//这里有疑惑，为什么析构时要加锁？
    pthread_mutex_destroy(&mutex);
  }
  void lock() { pthread_mutex_lock(&mutex); }
  void unlock() { pthread_mutex_unlock(&mutex); }
  pthread_mutex_t *get() { return &mutex; }

 private:
  pthread_mutex_t mutex;

  // 友元类不受访问权限影响
 private:
  friend class Condition;
};

class MutexLockGuard : noncopyable {//用类的构造和析构完成加锁和解锁
 public:
  explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex) { mutex.lock(); }
  ~MutexLockGuard() { mutex.unlock(); }

 private:
  MutexLock &mutex;//类成员引用类型
};

class sem
{
public:
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)//注意pshared表示允许几个进程共享该信号量，一般设0用于进程内的多线程共享
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if (sem_init(&m_sem, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    bool wait()//调用成功时返回0，失败返回-1，用于判断调用函数是否成功
    {
        return sem_wait(&m_sem) == 0;
    }
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};