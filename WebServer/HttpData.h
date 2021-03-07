
#pragma once //为了避免同一个头文件被包含（include）多次，C/C++中有两种宏实现方式：一种是#ifndef方式；另一种是#pragma once方式。
            //在能够支持这两种方式的编译器上，二者并没有太大的区别。但两者仍然有一些细微的区别。

#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"
#include "sql_connection_pool.h"

class EventLoop;
class TimerNode;
class Channel;

enum ProcessState {
  STATE_PARSE_URI = 1,//统一资源标志符
  STATE_PARSE_HEADERS,
  STATE_RECV_BODY,
  STATE_ANALYSIS,
  STATE_FINISH
};

enum URIState {
  PARSE_URI_AGAIN = 1,
  PARSE_URI_ERROR,
  PARSE_URI_SUCCESS,
};

enum HeaderState {
  PARSE_HEADER_SUCCESS = 1,
  PARSE_HEADER_AGAIN,
  PARSE_HEADER_ERROR
};

enum AnalysisState { ANALYSIS_SUCCESS = 1, ANALYSIS_ERROR };

enum ParseState {
  H_START = 0,
  H_KEY,
  H_COLON,
  H_SPACES_AFTER_COLON,
  H_VALUE,
  H_CR,
  H_LF,
  H_END_CR,
  H_END_LF
};

enum ConnectionState { H_CONNECTED = 0, H_DISCONNECTING, H_DISCONNECTED };

enum HttpMethod { METHOD_POST = 1, METHOD_GET, METHOD_HEAD };

enum HttpVersion { HTTP_10 = 1, HTTP_11 };

class MimeType {//是设定某种扩展名的文件用一种应用程序来打开的方式类型，当该扩展名文件被访问的时候，浏览器会自动使用指定应用程序来打开。多用于指定一些客户端自定义的文件名，以及一些媒体文件打开方式。
 //MIME Type，也就是该资源的媒体类型。一般由Content-Type指定
 private:
  static void init();
  static std::unordered_map<std::string, std::string> mime;
  MimeType();
  MimeType(const MimeType &m);

 public:
  static std::string getMime(const std::string &suffix);

 private:
  static pthread_once_t once_control;
};
//若一个类 T 继承 std::enable_shared_from_this<T> ，则会为该类 T 提供成员函数： shared_from_this 。 
//当 T 类型对象 t 被一个为名为 pt 的 std::shared_ptr<T> 类对象管理时，调用 T::shared_from_this 成员函数，将会返回一个新的 std::shared_ptr<T> 对象，它与 pt 共享 t 的所有权。
//当类A被share_ptr管理，且在类A的成员函数里需要把当前类对象作为参数传给其他函数时，就需要传递一个指向自身的share_ptr。
class HttpData : public std::enable_shared_from_this<HttpData> {
 public:
  HttpData(EventLoop *loop, int connfd,connection_pool* sqlpool);
  ~HttpData() { close(fd_); }
  void reset();
  void seperateTimer();
  void linkTimer(std::shared_ptr<TimerNode> mtimer) {
    // shared_ptr重载了bool, 但weak_ptr没有
    timer_ = mtimer;
  }
  std::shared_ptr<Channel> getChannel() { return channel_; }
  EventLoop *getLoop() { return loop_; }
  void handleClose();
  void newEvent();

 private:
  EventLoop *loop_;
  MYSQL *mysql;
  connection_pool* m_connPool;//sql连接池
  connectionRAII mysqlcon;//用于连接池的初始化与释放
  std::shared_ptr<Channel> channel_;
  int fd_;
  std::string inBuffer_;
  std::string outBuffer_;
  bool error_;
  ConnectionState connectionState_;

  HttpMethod method_;
  HttpVersion HTTPVersion_;
  std::string fileName_;
  std::string path_;
  int nowReadPos_;
  ProcessState state_;
  ParseState hState_;
  URIState findState_;
  bool keepAlive_;
  std::map<std::string, std::string> headers_;
  std::weak_ptr<TimerNode> timer_;

  void handleRead();
  void handleWrite();
  void handleConn();
  void handleError(int fd, int err_num, std::string short_msg);
  URIState parseURI();
  HeaderState parseHeaders();
  AnalysisState analysisRequest();
  MYSQL_RES mysql_queryresult(connection_pool *connPool,string querystr);
  void mysql_insertresult(connection_pool *connPool,string querystr);
  void getallmessage(int fd,string short_msg,string short_msg2,string userid);
  string findkey(string startkey,int pos,string endkey);
};