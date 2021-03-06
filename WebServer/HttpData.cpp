
#include "HttpData.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <iostream>
#include "Channel.h"
#include "EventLoop.h"
#include "Util.h"
#include "time.h"
#include <fstream>
#include <string>
#include <sstream>
#include <mysql/mysql.h>

using namespace std;
//宏初始化pthread_once_t控制变量
//pthread_once_t once = PTHREAD_ONCE_INIT; 
pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;

const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;              // ms
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000;  // ms

char favicon[555] = {//这是一个图标
    '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',
    '\x0',  '\x0',  '\xD',  'I',    'H',    'D',    'R',    '\x0',  '\x0',
    '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10', '\x8',  '\x6',  '\x0',
    '\x0',  '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',
    '\x19', 't',    'E',    'X',    't',    'S',    'o',    'f',    't',
    'w',    'a',    'r',    'e',    '\x0',  'A',    'd',    'o',    'b',
    'e',    '\x20', 'I',    'm',    'a',    'g',    'e',    'R',    'e',
    'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C', '\x0',  '\x0',
    '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94',
    '\x93', '9',    'H',    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D',
    'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',    '\x22', '\x1E', '\xA0',
    'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA',
    'J',    '\x9A', '\x80', '\x8',  'A',    '\xB4', 'q',    '\x85', 'X',
    '\x89', 'G',    '\xB0', 'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6',
    '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',
    '\xC1', 'F',    '\xB4', '\x15', '\xCF', '\x22', 'X',    '\x98', '\xB',
    'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',
    '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF',
    '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8', '\xD5', 'j',    'H',
    '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF',
    '\xF7', '\x2F', '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7',
    '\xAA', '\xD5', '\xEA', '\xAF', 'I',    '5',    'F',    '\xAA', 'T',
    '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5',
    'r',    '9',    'd',    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',
    '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',    '\xA7',
    '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A',
    '\xC6', '\x16', '\x82', '\x20', 'X',    'R',    '\x14', 'E',    '6',
    'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',
    'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',
    '\x9E', '\xBC', '\x9',  'A',    '\x7C', '\x3E', '\x1F', '\x83', 'D',
    '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',
    '\xE5', '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',
    '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g',    '\x98', '\xE9',
    '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8',
    '\xBD', '\x15', '\xE1', 'i',    '\xB7', 'C',    '\xAB', '\xEA', 'x',
    '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF',
    '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',
    '\x0',  'i',    '6',    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84',
    '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B', '7',
    'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9',
    'd',    '\x2',  'B',    '\xAF', '\x85', '2',    'f',    'F',    '\xBA',
    '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7',
    '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7',  '\x93',
    '\xA2', '\x28', '\xA0', 'S',    'j',    '\xB1', '\xB8', '\xDF', '\x29',
    '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',
    'j',    'P',    '\x40', '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',
    '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7', '\xA',  '\x26', '\xB6',
    '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
    '\xA',  '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27',
    '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB', '\xD6', 'G',
    '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2', '\xDD', '\xA0', '\x60',
    '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',
    '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',    '\xAC',
    '\xFE', '\xF2', '\x91', '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8',
    '\x87', '\xB7', 'X',    '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N',
    'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
    '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D',
    '\x7F', '\x9',  '2',    '\x95', '\xBF', '\x1F', '\xDB', '\xD7', 'c',
    '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J',    '\xC3',
    '\x87', '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A',
    'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',  '\x0',  'I',    'E',
    'N',    'D',    '\xAE', 'B',    '\x60', '\x82',
};

void MimeType::init() {
  mime[".html"] = "text/html";
  mime[".avi"] = "video/x-msvideo";
  mime[".bmp"] = "image/bmp";
  mime[".c"] = "text/plain";
  mime[".doc"] = "application/msword";
  mime[".gif"] = "image/gif";
  mime[".gz"] = "application/x-gzip";
  mime[".htm"] = "text/html";
  mime[".ico"] = "image/x-icon";
  mime[".jpg"] = "image/jpeg";
  mime[".png"] = "image/png";
  mime[".txt"] = "text/plain";
  mime[".mp3"] = "audio/mp3";
  mime["default"] = "text/html";
}

void HttpData::mysql_queryresult(string &querystr)
{
    if(mysql==NULL)
    {
      //LOG<<"mysql is NULL";
    }
    if (mysql_query(mysql, querystr.c_str()))
    {
      LOG<<"SELECT error:"<<mysql_error(mysql)<<"\n";
    }
}

void HttpData::mysql_insertresult(string &insertstr)
{
    if(mysql==NULL)
    {
      //LOG<<"mysql is NULL";
    }
    if (mysql_query(mysql, insertstr.c_str()))
    {
      LOG<<"INSERT error:"<<mysql_error(mysql)<<"\n";
    }
}

void readFileIntoString(char * filename,string &buf)
{
    buf.reserve(buf.capacity()+10000);
    ifstream ifile(filename);
    char ch;
    while(ifile.get(ch)) buf+=ch;
}

void HttpData::sendmessage(int fd, string short_msg)
{
  string header_buff;
  char * fn=(char*)short_msg.c_str();//这里如果用VS可能要来个强制转换(char*)"a.txt"
  string buf;
  readFileIntoString(fn,buf);
  header_buff += "HTTP/1.1 200 OK\r\n";
  header_buff += "Content-Type: text/html\r\n";
  header_buff += "Connection: keep-alive\r\n";
  header_buff += "Content-Length: " + to_string(buf.size()) + "\r\n";
  header_buff += "Server: autoli's Web Server\r\n";
  header_buff += "\r\n";
  outBuffer_+=header_buff;
  outBuffer_+=buf;
}

void HttpData::getallmessage(int fd,string short_msg,string short_msg2,string userid)
{
  char * fn=(char*)short_msg.c_str();//这里如果用VS可能要来个强制转换(char*)"a.txt"
  char * fn2=(char*)short_msg2.c_str();
  string send_buff;
  readFileIntoString(fn,send_buff);
  string header_buff;
  string sql_query="SELECT web_id,web_name,log_name,log_pass,log_des FROM infos where user_id=";
  sql_query += "'"+ userid +"'";
  mysql_queryresult(sql_query);
  //从表中检索完整的结果集
  SQLRES_RAII res(mysql);
  MYSQL_RES* result = res.get();
  //返回结果集中的列数
  int num_fields = mysql_num_fields(result);
  //返回所有字段结构的数组
  MYSQL_FIELD *fields = mysql_fetch_fields(result);
  //从结果集中获取下一行，将对应的用户名和密码，存入map中
  while (MYSQL_ROW row = mysql_fetch_row(result))
  {
    send_buff+="{";
    for(int i=0;i<num_fields;i++)
    {
      send_buff +=fields[i].name;
      send_buff +=":'";
      send_buff +=row[i];
      send_buff +="',";
    }
    send_buff+="},\r\n";      
  };
  readFileIntoString(fn2,send_buff);
  header_buff += "HTTP/1.1 " + to_string(200) + "OK"+"\r\n";
  header_buff += "Content-Type: text/html\r\n";
  header_buff += "Connection: keep-alive\r\n";
  header_buff += "Content-Length: " + to_string(send_buff.size()) + "\r\n";
  header_buff += "Server: autoli's Web Server\r\n";
  header_buff += "\r\n";
  outBuffer_+=header_buff;
  outBuffer_+=send_buff;
  // sprintf(output_buff, "%s", header_buff.c_str());
  // writen(fd, output_buff, strlen(output_buff));
  // sprintf(output_buff, "%s",send_buff.c_str());
  // writen(fd, output_buff, strlen(output_buff));
}

std::string MimeType::getMime(const std::string &suffix) {
  pthread_once(&once_control, MimeType::init);//该线程执行一次初始化
  if (mime.find(suffix) == mime.end())//寻找前缀
    return mime["default"];
  else
    return mime[suffix];
}


HttpData::HttpData(EventLoop *loop, int connfd,connection_pool* sqlpool)
    : loop_(loop),
      channel_(new Channel(loop, connfd)),
      fd_(connfd),
      error_(false),
      connectionState_(H_CONNECTED),
      method_(METHOD_GET),
      HTTPVersion_(HTTP_11),
      nowReadPos_(0),
      state_(STATE_PARSE_URI),
      hState_(H_START),
      keepAlive_(false),
      m_connPool(sqlpool),
      mysqlcon(&mysql,m_connPool) {//将读写时间与Httpdata事件绑定
  // loop_->queueInLoop(bind(&HttpData::setHandlers, this));
  channel_->setReadHandler(bind(&HttpData::handleRead, this));
  channel_->setWriteHandler(bind(&HttpData::handleWrite, this));
  channel_->setConnHandler(bind(&HttpData::handleConn, this));
}

void HttpData::reset() {
  // inBuffer_.clear();
  fileName_.clear();
  path_.clear();
  nowReadPos_ = 0;
  state_ = STATE_PARSE_URI;
  hState_ = H_START;
  headers_.clear();
  // keepAlive_ = false;
  if (timer_.lock()) {//尝试调用Lock()时才会有可能临时拥有对象
    shared_ptr<TimerNode> my_timer(timer_.lock());
    my_timer->clearReq();
    timer_.reset();
  }
}

void HttpData::seperateTimer() {
  // cout << "seperateTimer" << endl;
  if (timer_.lock()) {
    shared_ptr<TimerNode> my_timer(timer_.lock());
    my_timer->clearReq();
    timer_.reset();
  }
}

void HttpData::handleRead() {
  __uint32_t &events_ = channel_->getEvents();
  do {
    bool zero = false;
    int read_num = readn(fd_, inBuffer_, zero);
    
    //LOG << "Request: " << inBuffer_;
    if (connectionState_ == H_DISCONNECTING) {
      inBuffer_.clear();
      break;
    }
    if (read_num < 0) {
      perror("1");
      error_ = true;
      handleError(fd_, 400, "Bad Request");
      break;
    }
    else if (zero) {
      // 有请求出现但是读不到数据，可能是Request
      // Aborted，或者来自网络的数据没有达到等原因
      // 最可能是对端已经关闭了，统一按照对端已经关闭处理
      // error_ = true;
      connectionState_ = H_DISCONNECTING;
      if (read_num == 0) {
        // error_ = true;
        break;
      }
      // cout << "readnum == 0" << endl;
    }

    if (state_ == STATE_PARSE_URI) {
      URIState flag = this->parseURI();
      if (flag == PARSE_URI_AGAIN)
        break;
      else if (flag == PARSE_URI_ERROR) {
        perror("2");
        //LOG << "FD = " << fd_ << "," << inBuffer_ << "******";
        inBuffer_.clear();
        error_ = true;
        handleError(fd_, 400, "Bad Request");
        break;
      } else
        state_ = STATE_PARSE_HEADERS;
    }
    if (state_ == STATE_PARSE_HEADERS) {
      HeaderState flag = this->parseHeaders();
      if (flag == PARSE_HEADER_AGAIN)
        break;
      else if (flag == PARSE_HEADER_ERROR) {
        perror("3");
        error_ = true;
        handleError(fd_, 400, "Bad Request");
        break;
      }
      if (method_ == METHOD_POST) {
        // POST方法准备
        state_ = STATE_RECV_BODY;
      } else {
        state_ = STATE_ANALYSIS;
      }
    }
    if (state_ == STATE_RECV_BODY) {
      int content_length = -1;
      if (headers_.find("Content-length") != headers_.end()) {
        content_length = stoi(headers_["Content-length"]);
      } 
      else if(headers_.find("Content-Length") != headers_.end())
      {
         content_length = stoi(headers_["Content-Length"]);
      }
      else {
        // cout << "(state_ == STATE_RECV_BODY)" << endl;
        error_ = true;
        handleError(fd_, 400, "Bad Request: Lack of argument (Content-length)");
        break;
      }
      if (static_cast<int>(inBuffer_.size()) < content_length) break;
      state_ = STATE_ANALYSIS;
    }
    
    if (state_ == STATE_ANALYSIS) {
      AnalysisState flag = this->analysisRequest();
      if (flag == ANALYSIS_SUCCESS) {
        int pos=inBuffer_.find("\r\n");
        inBuffer_=inBuffer_.substr(pos+2);
        state_ = STATE_FINISH;
        break;
      } else {
        // cout << "state_ == STATE_ANALYSIS" << endl;
        error_ = true;
        break;
      }
    }
  } while (false);
  if (!error_) {
    if (outBuffer_.size() > 0) {
      handleWrite();
      // events_ |= EPOLLOUT;
    }
    // error_ may change
    if (!error_ && state_ == STATE_FINISH) {
      this->reset();
      if (inBuffer_.size() > 0) {
        if (connectionState_ != H_DISCONNECTING) handleRead();
      }
      // if ((keepAlive_ || inBuffer_.size() > 0) && connectionState_ ==
      // H_CONNECTED)
      // {
      //     this->reset();
      //     events_ |= EPOLLIN;
      // }
    } else if (!error_ && connectionState_ != H_DISCONNECTED)
      events_ |= EPOLLIN;
  }
}

void HttpData::handleWrite() {
  if (!error_ && connectionState_ != H_DISCONNECTED) {
    __uint32_t &events_ = channel_->getEvents();
    if (writen(fd_, outBuffer_) < 0) {
      perror("writen");
      events_ = 0;
      error_ = true;
    }
    if (outBuffer_.size() > 0) events_ |= EPOLLOUT;
  }
}

void HttpData::handleConn() {
  seperateTimer();
  __uint32_t &events_ = channel_->getEvents();
  if (!error_ && connectionState_ == H_CONNECTED) {
    if (events_ != 0) {
      int timeout = DEFAULT_EXPIRED_TIME;
      if (keepAlive_) timeout = DEFAULT_KEEP_ALIVE_TIME;
      if ((events_ & EPOLLIN) && (events_ & EPOLLOUT)) {
        events_ = __uint32_t(0);//清除
        events_ |= EPOLLOUT;
      }
      // events_ |= (EPOLLET | EPOLLONESHOT);
      events_ |= EPOLLET;
      loop_->updatePoller(channel_, timeout);

    } else if (keepAlive_) {
      events_ |= (EPOLLIN | EPOLLET);
      // events_ |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
      int timeout = DEFAULT_KEEP_ALIVE_TIME;
      loop_->updatePoller(channel_, timeout);
    } else {
      //要测试短连接，需要把下面注释的代码去掉注释
      // cout << "close normally" << endl;
      loop_->shutdown(channel_);
      loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
      //events_ |= (EPOLLIN | EPOLLET);
      // // events_ |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
      //int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
      //loop_->updatePoller(channel_, timeout);
    }
  } else if (!error_ && connectionState_ == H_DISCONNECTING &&
             (events_ & EPOLLOUT)) {
    events_ = (EPOLLOUT | EPOLLET);
  } else {
    // cout << "close with errors" << endl;
    loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
  }
}

URIState HttpData::parseURI() {
  string &str = inBuffer_;
  string cop = str;
  // 读到完整的请求行再开始解析请求
  ssize_t pos = str.find("\r\n", nowReadPos_);
  if (pos < 0) {
    return PARSE_URI_AGAIN;
  }
  // 去掉请求行所占的空间，节省空间
  string request_line = str.substr(0, pos);

  if (str.size() > pos + 2)
    str = str.substr(pos + 2);
  else
    str.clear();
  // Method
  int posGet = request_line.find("GET");
  int posPost = request_line.find("POST");
  int posHead = request_line.find("HEAD");

  if (posGet >= 0) {
    pos = posGet;
    method_ = METHOD_GET;
  } else if (posPost >= 0) {
    pos = posPost;
    method_ = METHOD_POST;
  } else if (posHead >= 0) {
    pos = posHead;
    method_ = METHOD_HEAD;
  } else {
    return PARSE_URI_ERROR;
  }
  
  // filename
  pos = request_line.find("/", pos);
  if (pos < 0) {
    fileName_ = "index.html";
    HTTPVersion_ = HTTP_11;
    return PARSE_URI_SUCCESS;
  } else {
    size_t _pos = request_line.find(' ', pos);
    if (_pos < 0)
      return PARSE_URI_ERROR;
    else {
      if (_pos - pos > 1) {
        fileName_ = request_line.substr(pos + 1, _pos - pos - 1);
        size_t __pos = fileName_.find('?');
        if (__pos >= 0) {
          fileName_ = fileName_.substr(0, __pos);
        }
      }

      else
        fileName_ = "index.html";
    }
    pos = _pos;
  }
  
  // HTTP 版本号
  pos = request_line.find("/", pos);
  if (pos < 0)
    return PARSE_URI_ERROR;
  else {
    if (request_line.size() - pos <= 3)
      return PARSE_URI_ERROR;
    else {
      string ver = request_line.substr(pos + 1, 3);
      if (ver == "1.0")
        HTTPVersion_ = HTTP_10;
      else if (ver == "1.1")
        HTTPVersion_ = HTTP_11;
      else
        return PARSE_URI_ERROR;
    }
  }
  return PARSE_URI_SUCCESS;
}

HeaderState HttpData::parseHeaders() {
  string &str = inBuffer_;
  int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
  int now_read_line_begin = 0;
  bool notFinish = true;
  size_t i = 0;
  for (; i < str.size() && notFinish; ++i) {
    switch (hState_) {
      case H_START: {
        if (str[i] == '\n' || str[i] == '\r') break;
        hState_ = H_KEY;
        key_start = i;
        now_read_line_begin = i;
        break;
      }
      case H_KEY: {
        if (str[i] == ':') {
          key_end = i;
          if (key_end - key_start <= 0) return PARSE_HEADER_ERROR;
          hState_ = H_COLON;
        } else if (str[i] == '\n' || str[i] == '\r')
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_COLON: {
        if (str[i] == ' ') {
          hState_ = H_SPACES_AFTER_COLON;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_SPACES_AFTER_COLON: {
        hState_ = H_VALUE;
        value_start = i;
        break;
      }
      case H_VALUE: {
        if (str[i] == '\r') {
          hState_ = H_CR;
          value_end = i;
          if (value_end - value_start <= 0) return PARSE_HEADER_ERROR;
        } else if (i - value_start > 255)
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_CR: {
        if (str[i] == '\n') {
          hState_ = H_LF;
          string key(str.begin() + key_start, str.begin() + key_end);
          string value(str.begin() + value_start, str.begin() + value_end);
          headers_[key] = value;
          now_read_line_begin = i;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_LF: {
        if (str[i] == '\r') {
          hState_ = H_END_CR;
        } else {
          key_start = i;
          hState_ = H_KEY;
        }
        break;
      }
      case H_END_CR: {
        if (str[i] == '\n') {
          hState_ = H_END_LF;
        } else
          return PARSE_HEADER_ERROR;
        break;
      }
      case H_END_LF: {
        notFinish = false;
        key_start = i;
        now_read_line_begin = i;
        break;
      }
    }
  }
  i--;//好像多加了一次，这里要注意
  if (hState_ == H_END_LF) {
    str = str.substr(i);
    return PARSE_HEADER_SUCCESS;
  }
  str = str.substr(now_read_line_begin);
  return PARSE_HEADER_AGAIN;
}

string HttpData::findkey(string startkey,int pos,string endkey)
{
  string &str=inBuffer_;
  int startpos=str.find(startkey,pos);
  if(startpos==-1)
  {
    findState_=PARSE_URI_ERROR;
    return "";
  }
  int endpos=str.find(endkey,startpos);
  if(endpos==-1)
  {
    findState_=PARSE_URI_ERROR;
    return "";
  }
  string strres=str.substr(startpos+startkey.size(),endpos-startpos-startkey.size());
  str = str.substr(endpos);
  findState_=PARSE_URI_SUCCESS;
  return strres;
}

AnalysisState HttpData::analysisRequest() {
  // cout<<fileName_<<endl;
  // cout<<inBuffer_<<endl;
  if (headers_.find("Connection") != headers_.end() &&
  (headers_["Connection"] == "Keep-Alive" ||
    headers_["Connection"] == "keep-alive")) {
    keepAlive_ = true;
  }
  if (method_ == METHOD_POST) {
    if(fileName_ == "login")//密码登陆
    {
      string name=findkey("user=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string password=findkey("password=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string sql_query="SELECT user_id FROM user where username=";
      sql_query += "'"+ name +"'";
      sql_query +="and ";
      sql_query+="passwd=";
      sql_query += "'"+ password +"'";
      //LOG<<sql_query;
      mysql_queryresult(sql_query);
      SQLRES_RAII res(mysql);
      MYSQL_RES* result = res.get();
      //返回结果集中的列数
      int num_fields = mysql_num_fields(result);   
      if(mysql_num_rows(result))
      { 
        string temp;
        //从结果集中获取下一行，将对应的用户名和密码，存入map中
        while (MYSQL_ROW row = mysql_fetch_row(result))
        {
          temp=row[0];
        }
        getallmessage(fd_,"html/welcome.txt","html/welcome2.txt",temp);
      }
      else
      {
        sendmessage(fd_,"html/logError.html");
      }
      return ANALYSIS_SUCCESS;
    }
    if(fileName_=="register")
    {
      sendmessage(fd_,"html/register.html");
      return ANALYSIS_SUCCESS;
    }
    if(fileName_=="registerpage")
    {
      string name=findkey("user=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string password=findkey("password=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string sql_query="SELECT * FROM user where username=";
      sql_query +="'"+ name +"'";
      mysql_queryresult(sql_query);
      SQLRES_RAII res(mysql);
      MYSQL_RES* result = res.get();
      // MYSQL_RES  tmp = mysql_queryresult(m_connPool,sql_query);
      // //从表中检索完整的结果集
      // MYSQL_RES* result = &tmp;
      if(mysql_num_rows(result))
      {
        sendmessage(fd_,"html/registerError.html");
        return ANALYSIS_SUCCESS;
      }
      string insert_query="INSERT INTO user(username,passwd) values(";
      insert_query += "'";
      insert_query +=name;
      insert_query +="',";
      insert_query += "'";
      insert_query +=password;
      insert_query +="'";
      insert_query +=")";
      //LOG<<insert_query;
      mysql_insertresult(insert_query);
      sendmessage(fd_,"html/registersuccess.html");
      return ANALYSIS_SUCCESS;
    }
    if(fileName_=="addData")
    {
      sendmessage(fd_,"html/addData.html");
      return ANALYSIS_SUCCESS;
    }
    if(fileName_=="addDatapage")
    {
      string webtext=findkey("webtext=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string webname=findkey("webname=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string webpass=findkey("webpass=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string webdes=findkey("webdes=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string name=findkey("user=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string password=findkey("password=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string sql_query="SELECT user_id FROM user where username='"+name+"' and passwd='"+ password+"'";//注意此处自带引号，这是与html的form enctype="text/plain"有关
      //LOG<<sql_query;
      mysql_queryresult(sql_query);
      SQLRES_RAII res(mysql);
      MYSQL_RES* result = res.get();
      // MYSQL_RES  tmp = mysql_queryresult(m_connPool,sql_query);
      
      // //从表中检索完整的结果集
      // MYSQL_RES* result = &tmp;
      string temp;
      while (MYSQL_ROW row = mysql_fetch_row(result))
      {
          temp=row[0];
      }
      string insert_query="INSERT INTO infos(user_id,web_name,log_name,log_pass,log_des) values(";
      insert_query += "'";
      insert_query +=temp;
      insert_query +="',";
      insert_query += "'";
      insert_query +=webtext;
      insert_query +="',";
      insert_query += "'";
      insert_query +=webname;
      insert_query +="',";
      insert_query += "'";
      insert_query +=webpass;
      insert_query +="',";
      insert_query += "'";
      insert_query +=webdes;
      insert_query +="'";
      insert_query +=")";
      //LOG<<insert_query;
      mysql_insertresult(insert_query);
      getallmessage(fd_,"html/welcome.txt","html/welcome2.txt",temp);
      return ANALYSIS_SUCCESS;
    }

    if(fileName_ == "DeleteText" )
    {
      string name=findkey("user=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string password=findkey("password=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      
      string webid=findkey("web_id=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }

      string sql_query="SELECT user_id FROM user where username='"+name+"' and passwd='"+ password+"'";
      mysql_queryresult(sql_query);
      SQLRES_RAII res(mysql);
      MYSQL_RES* result = res.get();
      // MYSQL_RES  tmp = mysql_queryresult(m_connPool,sql_query);
      // //从表中检索完整的结果集
      // MYSQL_RES* result = &tmp;
      //返回结果集中的列数
      int num_fields = mysql_num_fields(result);
    
      if(!mysql_num_rows(result))
      { 
        sendmessage(fd_,"html/logtimeout.html");
      }

      string temp; 
      while (MYSQL_ROW row = mysql_fetch_row(result))
      {
        temp=row[0];
      }
      
      if(!mysql_num_rows(result))
      { 
        sendmessage(fd_,"html/logtimeout.html");
      }

      string delete_query="DELETE FROM infos where web_id=";
      delete_query += "'";
      delete_query +=webid;
      delete_query +="'";
      //LOG<<delete_query;
      mysql_insertresult(delete_query);
      
      getallmessage(fd_,"html/welcome.txt","html/welcome2.txt",temp);
      return ANALYSIS_SUCCESS;
    }
    if(fileName_ == "UpdateText" )
    {
      string name=findkey("user=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string password=findkey("password=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      
      string webid=findkey("web_id=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }

      string webtext=findkey("网址=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string webname=findkey("用户名=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string webpass=findkey("密码=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
      string webdes=findkey("说明=",0,"\r\n");
      if(findState_== PARSE_URI_ERROR)
      {
        return ANALYSIS_ERROR;
      }
     

      string sql_query="SELECT user_id FROM user where username='"+name+"' and passwd='"+ password+"'";
      // MYSQL_RES  tmp = mysql_queryresult(m_connPool,sql_query);
      // //从表中检索完整的结果集
      // MYSQL_RES* result = &tmp;
      mysql_queryresult(sql_query);
      SQLRES_RAII res(mysql);
      MYSQL_RES* result = res.get();
      //返回结果集中的列数
      int num_fields = mysql_num_fields(result);

      if(!mysql_num_rows(result))
      { 
        sendmessage(fd_,"html/logtimeout.html");
      }

      string temp; 
      while (MYSQL_ROW row = mysql_fetch_row(result))
      {
        temp=row[0];
      }
      
      string update_query="UPDATE infos SET web_name=";
      update_query += "'";
      update_query +=webtext;
      update_query +="'";
      update_query += ",log_name='";
      update_query +=webname;
      update_query +="'";
      update_query += ",log_pass='";
      update_query +=webpass;
      update_query +="'";
      update_query += ",log_des='";
      update_query +=webdes;
      update_query +="' ";
      update_query +=" WHERE web_id=";
      update_query += "'";
      update_query +=webid;
      update_query +="'";
      update_query +=" AND user_id=";
      update_query += "'";
      update_query +=temp;
      update_query +="'";
      
      mysql_insertresult(update_query);
      
      getallmessage(fd_,"html/welcome.txt","html/welcome2.txt",temp);
      return ANALYSIS_SUCCESS;  
    }
    if(fileName_=="exitall")
    {
      sendmessage(fd_,"html/exit.html");
      return ANALYSIS_SUCCESS;  
    }
    return ANALYSIS_SUCCESS;

  } else if (method_ == METHOD_GET || method_ == METHOD_HEAD) {
    string header;
    header += "HTTP/1.1 200 OK\r\n";
    if (headers_.find("Connection") != headers_.end() &&
        (headers_["Connection"] == "Keep-Alive" ||
         headers_["Connection"] == "keep-alive")) {
      keepAlive_ = true;
      header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" +
                to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
    }
    int dot_pos = fileName_.find('.');
    string filetype;
    if (dot_pos < 0)
      filetype = MimeType::getMime("default");
    else
      filetype = MimeType::getMime(fileName_.substr(dot_pos));

    // echo test
    if (fileName_ == "hello") {
      outBuffer_ += "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nConnection: Close\r\nContent-Length:11 \r\n\r\nHello World";
      return ANALYSIS_SUCCESS;
    }
    if (fileName_ == "favicon.ico") {
      header += "Content-Type: image/png\r\n";
      header += "Content-Length: " + to_string(sizeof favicon) + "\r\n";
      header += "Server: autoli's Web Server\r\n";

      header += "\r\n";
      outBuffer_ += header;
      outBuffer_ += string(favicon, favicon + sizeof favicon);
      
      return ANALYSIS_SUCCESS;
    }
    if(fileName_ == "login"||fileName_ == "index.html"||fileName_ == "index")
    {
      //这里加了模拟访问数据库
      string sql_query="SELECT user_id FROM user where username= '123' and passwd='456'";
      mysql_queryresult(sql_query);
      SQLRES_RAII res(mysql);
      sendmessage(fd_,"html/log.html");
      return ANALYSIS_SUCCESS;
    }

    // if(fileName_ == "index.html"||fileName_ == "index")
    // {
    //   string sql_query="SELECT user_id FROM user where username= '123' and passwd='456'";
    //   mysql_queryresult(sql_query);
    //   MYSQL_RES *result = mysql_store_result(mysql);
    //   mysql_free_result(result);
    //   outBuffer_ = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nConnection: Close\r\nContent-Length:11 \r\n\r\nHello World";
    //   return ANALYSIS_SUCCESS;
    // }
    
    struct stat sbuf;
    if (stat(fileName_.c_str(), &sbuf) < 0) {
      header.clear();
      handleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    header += "Content-Type: " + filetype + "\r\n";
    header += "Content-Length: " + to_string(sbuf.st_size) + "\r\n";
    header += "Server: autoli's Web Server\r\n";
    // 头部结束
    header += "\r\n";
    outBuffer_ += header;

    if (method_ == METHOD_HEAD) return ANALYSIS_SUCCESS;

    int src_fd = open(fileName_.c_str(), O_RDONLY, 0);
    if (src_fd < 0) {
      outBuffer_.clear();
      handleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    void *mmapRet = mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    close(src_fd);
    if (mmapRet == (void *)-1) {
      munmap(mmapRet, sbuf.st_size);
      outBuffer_.clear();
      handleError(fd_, 404, "Not Found!");
      return ANALYSIS_ERROR;
    }
    char *src_addr = static_cast<char *>(mmapRet);
    outBuffer_ += string(src_addr, src_addr + sbuf.st_size);
    munmap(mmapRet, sbuf.st_size);
    return ANALYSIS_SUCCESS;
  }
  return ANALYSIS_ERROR;
}




void HttpData::handleError(int fd, int err_num, string short_msg) {//发送错误
  short_msg = " " + short_msg;
  char send_buff[4096];
  string body_buff, header_buff;
  body_buff += "<html> <meta charset=\"UTF-8\"> <title>哎~出错了</title>";
  body_buff += "<body bgcolor=\"ffffff\">";
  body_buff += to_string(err_num) + short_msg;
  body_buff += "<hr><em> autoli's Web Server</em>\n</body></html>";

  header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
  header_buff += "Content-Type: text/html\r\n";
  header_buff += "Connection: Close\r\n";
  header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
  header_buff += "Server: autoli's Web Server\r\n";
  ;
  header_buff += "\r\n";
  // 错误处理不考虑writen不完的情况
  sprintf(send_buff, "%s", header_buff.c_str());
  writen(fd, send_buff, strlen(send_buff));
  sprintf(send_buff, "%s", body_buff.c_str());
  writen(fd, send_buff, strlen(send_buff));
}


void HttpData::handleClose() {
  connectionState_ = H_DISCONNECTED;
  shared_ptr<HttpData> guard(shared_from_this());
  loop_->removeFromPoller(channel_);
}

void HttpData::newEvent() {
  channel_->setEvents(DEFAULT_EVENT);
  loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}
