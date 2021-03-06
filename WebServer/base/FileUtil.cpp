
#include "FileUtil.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

AppendFile::AppendFile(string filename) : fp_(fopen(filename.c_str(), "ae")) {
  // 用户提供缓冲区
  setbuffer(fp_, buffer_, sizeof buffer_);
  //在打开文件流后, 读取内容之前, 调用setbuffer()可用来设置文件流的缓冲区. 参数stream 为指定的文件流, 参数buf 指向自定的缓冲区起始地址, 参数size 为缓冲区大小.
}

AppendFile::~AppendFile() { fclose(fp_); }

void AppendFile::append(const char* logline, const size_t len) {
  size_t n = this->write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = this->write(logline + n, remain);
    if (x == 0) {
      int err = ferror(fp_);
      if (err) fprintf(stderr, "AppendFile::append() failed !\n");
      break;
    }
    n += x;
    remain = len - n;
  }
}

//fflush()的作用是用来刷新缓冲区，
//fflush(stdin)刷新标准输入缓冲区，把输入缓冲区里的东西丢弃； 
//fflush(stdout)刷新标准输出缓冲区，把输出缓冲区里的东西强制打印到标准输出设备上。
void AppendFile::flush() { fflush(fp_); }

//fwrite的线程不安全版本
//函数原型：size_t 
//fwrite(const void* buffer, size_t size, size_t count, FILE * stream);
//含义：将指定缓冲区中的内容（buffer），部分或全部写入到指定的文件中（stream）
size_t AppendFile::write(const char* logline, size_t len) {
  return fwrite_unlocked(logline, 1, len, fp_);
}