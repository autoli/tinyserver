// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Thread.h"
#include <assert.h>
#include <errno.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include "CurrentThread.h"


#include <iostream>
using namespace std;

namespace CurrentThread {
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "default";
}
//各个进程独立，所以会有不同进程中线程号相同节的情况。
//那么这样就会存在一个问题，我的进程p1中的线程pt1要与进程p2中的线程pt2通信怎么办，
//进程id不可以，线程id又可能重复，所以这里会有一个真实的线程id唯一标识，tid。
//glibc没有实现gettid的函数，所以我们可以通过linux下的系统调用syscall(SYS_gettid)来获得。
pid_t gettid() { return static_cast<pid_t>(::syscall(SYS_gettid)); }

//函数说明:snprintf
// 最多从源串中拷贝size－1个字符到目标串中，然后再在后面加一个0。所以如果目标串的大小为size的话，将不会溢出。
// 函数返回值:
// 若成功则返回欲写入的字符串长度，若出错则返回负值。
//这里是把唯一标识符写入字符串中保存
void CurrentThread::cacheTid() {
  if (t_cachedTid == 0) {
    t_cachedTid = gettid();
    t_tidStringLength =
        snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
  }
}

// 为了在线程中保留name,tid这些数据
struct ThreadData {
  typedef Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  string name_;
  pid_t* tid_;
  CountDownLatch* latch_;

  ThreadData(const ThreadFunc& func, const string& name, pid_t* tid,
             CountDownLatch* latch)
      : func_(func), name_(name), tid_(tid), latch_(latch) {}

  void runInThread() {
    *tid_ = CurrentThread::tid();
    tid_ = NULL;
    latch_->countDown();
    latch_ = NULL;

    CurrentThread::t_threadName = name_.empty() ? "Thread" : name_.c_str();
    prctl(PR_SET_NAME, CurrentThread::t_threadName); //返回调用进程的进程名字给参数t_threadName; 
   
    func_();
    CurrentThread::t_threadName = "finished";
  }
};

void* startThread(void* obj) {
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return NULL;
}

Thread::Thread(const ThreadFunc& func, const string& n)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(func),
      name_(n),
      latch_(1) {
  setDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) pthread_detach(pthreadId_);
}

void Thread::setDefaultName() {
  if (name_.empty()) {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread");
    name_ = buf;
  }
}

void Thread::start() {
  assert(!started_);
  started_ = true;
  ThreadData* data = new ThreadData(func_, name_, &tid_, &latch_);
  if (pthread_create(&pthreadId_, NULL, &startThread, data)) {
    started_ = false;
    delete data;
  } else {
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}