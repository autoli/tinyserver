
#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    : mutex_(), condition_(mutex_), count_(count) {}

void CountDownLatch::wait() {
  MutexLockGuard lock(mutex_);//这个mutex主要是用来保证pthread_cond_wait的并发性
  while (count_ > 0) condition_.wait();
  //这个while要特别说明一下，单个pthread_cond_wait功能很完善，为何这里要有一个while (head == NULL)呢？
  //因为pthread_cond_wait里的线程可能会被意外唤醒，如果这个时候head != NULL，则不是我们想要的情况。
  //这个时候，应该让线程继续进入pthread_cond_wait
	//pthread_cond_wait会先解除之前的pthread_mutex_lock锁定的mtx，然后阻塞在等待对列里休眠，
  //直到再次被唤醒（大多数情况下是等待的条件成立而被唤醒，唤醒后，该进程会先锁定先pthread_mutex_lock(&mtx);，再读取资源
  //说明：
  //在调用pthread_cond_wait()前必须由本线程加锁（pthread_mutex_lock()），
  //而在更新条件等待队列以前，mutex保持锁定状态，并在线程挂起进入等待前解锁。
  //在条件满足从而离开pthread_cond_wait()之前，mutex将被重新加锁，以与进入pthread_cond_wait()前的加锁动作对应。

}

void CountDownLatch::countDown() {
  MutexLockGuard lock(mutex_);
  --count_;
  if (count_ == 0) condition_.notifyAll();
}