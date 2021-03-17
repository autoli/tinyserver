#include "Server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include "Util.h"
#include "base/Logging.h"

Server::Server(EventLoop *loop, int threadNum, int port)//这里主要完成了两步，一是用loop初始化了线程池，二是初始化了acceptChannel
    : loop_(loop),
      threadNum_(threadNum),
      m_sql_num(100),
      eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
      started_(false),
      acceptChannel_(new Channel(loop_)),
      port_(port),
      listenFd_(socket_bind_listen(port_)) {//创建监听端口
  acceptChannel_->setFd(listenFd_);
  handle_for_sigpipe();
  //数据库初始化
  m_connPool = connection_pool::GetInstance();
  m_connPool->init("localhost", "root1", "123456", "web", 3306, m_sql_num, 0);//线程池的sql数量和日志是否关闭

  
  if (setSocketNonBlocking(listenFd_) < 0) {
    perror("set socket non block failed");
    abort();
  }

}

void Server::start() {
  eventLoopThreadPool_->start();//这一步初始化了numThreads_的线程池，何时启动里面的thread，线程池里面的线程一直在阻塞，一直等到threadFun通过条件变量才开始运行
  // acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
  acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));//在channel里面设置read函数，问题：这两个函数何时被调用？
  acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));//问题：这里为什么是handThisConn调用update？这个是每次有事件就触发，然后如果事件监听有变化就更新一下
  loop_->addToPoller(acceptChannel_, 0);//每一个Eventloop有一个epoll，将acceptChannel加入到poller里面监听
  started_ = true;
}

void Server::handNewConn() {
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(struct sockaddr_in));
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = 0;
  while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) {//此处的listenFd_是非阻塞的，如果返回值为-1，且errno == EAGAIN或errno == EWOULDBLOCK表示no connections没有新连接请求
    EventLoop *loop = eventLoopThreadPool_->getNextLoop();//从线程池里面拿一个loop
    //LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        //<< ntohs(client_addr.sin_port);
    // 限制服务器的最大并发连接数
    if (accept_fd >= MAXFDS) {
      close(accept_fd);
      continue;
    }
    // 设为非阻塞模式
    if (setSocketNonBlocking(accept_fd) < 0) {
      //LOG << "Set non block failed!";
      // perror("Set non block failed!");
      return;
    }

    setSocketNodelay(accept_fd);
    // setSocketNoLinger(accept_fd);

    shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd,m_connPool));//用loop和accept_fd初始化HTTPdata
    req_info->getChannel()->setHolder(req_info);//便于从channel获取httpdata对象，这里是因为Httpdata和channel有相互引用，所以不会析构掉httpdata,但是channel中的是weak_ptr
    loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));//将channel与httpdata绑定，加入队列中，queueInloop用来执行用户的某个回调函数，
    //这里的loop是如何触发读写的，当主线程把新连接分配给了某个SubReactor，该线程此时可能正阻塞在多路选择器(epoll)的等待中，怎么得知新连接的到来呢？这里使用了eventfd进行异步唤醒，线程会从epoll_wait中醒来，得到活跃事件，进行处理。
  }
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);//这里为什幺要重新设置一次
}