#include "server.h"
#include "log/log.h"
#include "error_code.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <bits/types/error_t.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

IOLoop * IOLoop::Instance() {
  static IOLoop instance;
  return &instance;
}

IOLoop::~IOLoop() {
  for (auto handler : handlers_) {
    if (handler.second != nullptr) {
      delete handler.second;
      handler.second = nullptr;
    }
  }
  int ret = close(epfd_);
  if (ret != 0) {
    PLOG_ERR("close err", ret, "epfd[%d]", epfd_);
    return;
  }
  PLOG_RUN("close succ epfd[%d]", epfd_);
  return;
}

void IOLoop::Start() {
  // !!!Attention: max_events如果过小,会造成事件丢失.
  // 例如，如果有 100 个事件同时发生，而 maxevents 的值只设置为 10
  // 那么只有前 10 个事件会被存储到 events 数组中，后面的 90 个事件会被丢失。
  // max_events应该设置成 系统的最大文件描述符数量
  constexpr uint64_t MAX_EVENTS = 10000;
  struct epoll_event events[MAX_EVENTS];
  while (true) {
    // -1 只没有事件一直阻塞
    const int ret = epoll_wait(epfd_, events, MAX_EVENTS, -1/*Timeout*/);
    if (ret < 0) {
      PLOG_ERR("epoll_wait err", ret, "errno[%d] errmsg[%s]", errno, strerror(errno));
      break;
    }

    const uint32_t nfds = ret;
    PLOG_RUN("epoll_wait return nfds[%u]", nfds);

    for (int i = 0; i < nfds; ++i) {
      const int fd = events[i].data.fd;
      if (handlers_.find(fd) == handlers_.end()) {
        PLOG_ERR("handler not exist", my_errorcode::kHandlerNotExist, "fd[%d] not in handler", fd);
        return;
        // continue;
      }
      const int ret = handlers_[fd]->Handle(events[i]);
      if (ret != 0) {
        PLOG_ERR("hander err", ret, "fd[%d]", fd);
        handlers_.erase(fd);
        continue;
      }
      PLOG_RUN("handler succ fd[%d]", fd);
    }
  }
}

int IOLoop::AddHandler(int fd, Handler* handler, unsigned int events) {
  handlers_[fd] = handler;

  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  // event.events = events | EPOLLET | EPOLLHUP | EPOLLERR;
  // event.events = events | EPOLLHUP | EPOLLERR;

  const int ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
  if (ret < 0) {
    PLOG_ERR("epoll_ctl EPOLL_CTL_ADD err", ret, "fd[%d] errno[%d] errmsg[%s]", fd, errno, strerror(errno));
    return ret;
  }

  PLOG_RUN("epoll_ctl EPOLL_CTL_ADD fd[%d] event[%u]", fd, static_cast<uint32_t>(event.events));
  return 0;
}

int IOLoop::ModifyHandlerIn(int fd) {
  PLOG_RUN("modify fd[%d] to in", fd);
  return ModifyHandler(fd, EPOLLIN | EPOLLET);
}

int IOLoop::ModifyHandlerOut(int fd) {
  PLOG_RUN("modify fd[%d] to out", fd);
  return ModifyHandler(fd, EPOLLOUT | EPOLLET);
}

int IOLoop::ModifyHandler(int fd, unsigned int events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  // event.events = events | EPOLLET | EPOLLHUP | EPOLLERR;

  const int ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
  if (ret < 0) {
    PLOG_ERR("epoll_ctl EPOLL_CTL_MOD err", ret, "fd[%d] errno[%d] errmsg[%s]", fd, errno, strerror(errno));
    return ret;
  }
  return 0;
}

int IOLoop::RemoveHandler(int fd) {
  // 将fd从epoll堆删除
  int ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
  if (ret < 0) {
    PLOG_ERR("epoll_ctl EPOLL_CTL_DEL err", ret, "fd[%d] errno[%d] errmsg[%s]", fd, errno, strerror(errno));
    return ret;
  }
  PLOG_RUN("epoll_ctl EPOLL_CTL_DEL fd[%d]", fd);

  ret = close(fd);
  if (ret < 0) {
    PLOG_ERR("close err", ret, "fd[%d]", fd);
    return ret;
  }
  PLOG_RUN("close fd[%d] succ", fd);

  Handler* handler = handlers_[fd];
  handlers_.erase(fd);
  delete handler;
  handler = nullptr;

  PLOG_RUN("delete handler succ");

  return 0;
}

int IOLoop::Init() {
  int ret = epoll_create1(EPOLL_CLOEXEC);  //flag=0 等价于epll_craete
  if (ret < 0) {
    PLOG_ERR("epoll_create1 err", ret, "epoll_create1 errno[%d] errmsg[%s]", errno, strerror(errno));
    return ret;
  }
  epfd_ = ret;

  ret = fcntl(epfd_, F_GETFL, 0);
  if (ret < 0) {
    PLOG_ERR("fcntl get err", ret, "epfd[%d] errno[%d] errmsg[%s]", epfd_, errno, strerror(errno));
    return ret;
  }

  int flags = ret;
  ret = fcntl(epfd_, F_SETFL, flags | O_NONBLOCK);
  if (ret < 0) {
    PLOG_ERR("fcntl set err", ret, "nonblock epfd[%d] errno[%d] errmsg[%s]", epfd_, errno, strerror(errno));
    return ret;
  }

  PLOG_RUN("create epfd[%d] succ", epfd_);
  return 0;
}

int EchoHandler::SendData(int fd) {
  if (not has_received_) {
    PLOG_RUN("nothing to send");
    return 0;
  }

  uint32_t sent_buffer_length = 0;
  while (sent_buffer_length < received_data_.length()) {
    constexpr uint32_t kOnceSendBufferSize = 10;
    const uint32_t sent_buffer_length_thistime =
        std::min(static_cast<uint32_t>(received_data_.length() - sent_buffer_length), kOnceSendBufferSize);
    // const int ret = send(fd, received_data_.data() + sent_buffer_length, sent_buffer_length_thistime, MSG_DONTWAIT);
    const int ret = send(fd, received_data_.data() + sent_buffer_length, sent_buffer_length_thistime, 0);
    if (ret == 0) {
      PLOG_ERR("send zero err", my_errorcode::kSendZero,
              "send fd[%d] zero has_sent_buffer_length[%u] close it", fd, sent_buffer_length);
      IOLoop::Instance()->RemoveHandler(fd);
      return my_errorcode::kSendZero;
    }

    if (ret < 0) {
      PLOG_ERR("send err", ret, "fd[%d] errno[%d] errmsg[%s]", fd, errno, strerror(errno));
      IOLoop::Instance()->RemoveHandler(fd);
      return ret;
    }
    const uint32_t sent_buffer_length_thistime_actual = ret;
    PLOG_RUN("send fd[%d] length[%u] this time", fd, sent_buffer_length_thistime_actual);
    sent_buffer_length += sent_buffer_length_thistime;
  }

  if (sent_buffer_length > received_data_.length()) {
    PLOG_ERR("send err", my_errorcode::kSendErr,
            "sent_length[%u] > received_length[%lu]", sent_buffer_length, received_data_.length());
    IOLoop::Instance()->RemoveHandler(fd);
    return my_errorcode::kSendErr;
  }

  PLOG_RUN("send fd[%d] data[%s]", fd, received_data_.c_str());
  has_received_ = false;
  IOLoop::Instance()->ModifyHandlerIn(fd);
  return 0;
}

int EchoHandler::RecvData(int fd) {
  has_received_ = true;
  received_data_.clear();
  // TODO(camxu): while recv
  while (true) {
    constexpr uint32_t kOnceRecvBufferSize = 10;
    std::array<char, kOnceRecvBufferSize> recv_buffer;
    // const int ret = recv(fd, &recv_buffer, kOnceRecvBufferSize, MSG_DONTWAIT);
    const int ret = recv(fd, &recv_buffer, kOnceRecvBufferSize, 0);
    /*
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 套接字上没有数据可读
                    // 可以将套接字重新添加到 epoll 实例中，以便监听未处理的数据
                    epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]);
                    */
    if (ret == 0) {
      PLOG_ERR("receive err", my_errorcode::kRecvZero, "receive data len[%d] is zero", ret);
      IOLoop::Instance()->RemoveHandler(fd);
      /*
       * 对端关闭连接：当对端调用 close 函数关闭连接时，服务器端会收到 EPOLLHUP 事件，表示对端已经关闭了连接。此时，服务器端需要关闭套接字，并从 epoll 实例中删除该文件描述符。
       * 连接超时：当连接超过一定时间没有收到数据时，服务器端可以认为连接已经失效，需要关闭套接字，并从 epoll 实例中删除该文件描述符。
       * 发生错误：当套接字发生错误时，服务器端会收到 EPOLLERR 事件，表示发生了错误。此时，服务器端需要关闭套接字，并从 epoll 实例中删除该文件描述符。
       * 服务器端主动关闭连接：当服务器端需要关闭连接时，可以调用 close 函数关闭套接字，并从 epoll 实例中删除该文件描述符。
       */
      return my_errorcode::kRecvZero;
    }
    // if (ret < 0 and (errno == EAGAIN or errno == EWOULDBLOCK)) {
    if (ret < 0 and (errno == EAGAIN or errno == EWOULDBLOCK)) {
      PLOG_RUN("recv complete by errno");
      continue;
    }
    if (ret < 0) {
      // TODO(camxu): 建设fd和client的映射，方便debug
      PLOG_ERR("received err", ret, "fd[%d] errmsg[%s]", fd, strerror(ret));
      // TODO(camxu): receive error then remove?
      IOLoop::Instance()->RemoveHandler(fd);
      return my_errorcode::kRecvErr;
    }

    const uint32_t recv_buffer_length_thistime = ret;
    PLOG_RUN("recv fd[%d] length[%u] this time", fd, recv_buffer_length_thistime);
    received_data_ += std::string(recv_buffer.begin(), recv_buffer.begin() + recv_buffer_length_thistime);

    if (recv_buffer_length_thistime < kOnceRecvBufferSize) {
      PLOG_RUN("recv finish");
      break;
    }
  }

  PLOG_RUN("receive from fd[%d] data[%s]", fd, received_data_.c_str());
  IOLoop::Instance()->ModifyHandlerOut(fd);
  return 0;
}

int EchoHandler::Handle(struct epoll_event event) {
  int fd = event.data.fd;
  /*
   * 在 Linux 中，当一个套接字发生错误时，内核会向 epoll 实例注册的文件描述符发送一个 EPOLLERR 事件，
   * 以通知应用程序发生了错误。当 epoll 实例接收到 EPOLLERR 事件时，应用程序需要根据具体的错误码来处理错误。
   * 在处理 EPOLLERR 事件时，应用程序可以使用 errno 变量来获取错误码。通常情况下，errno 变量的值为 0，表示没有错误发生。
   * 但是，当套接字发生错误时，errno 变量的值可能会被设置为相应的错误码。
   * 例如，当套接字的发送缓冲区已满时，send 函数将返回 -1，并设置 errno 变量为 EAGAIN 或 EWOULDBLOCK。
   * 当 epoll 实例接收到 EPOLLERR 事件时，应用程序可以检查 errno 变量的值，以确定错误的原因。
   * 需要注意的是，EPOLLERR 事件通常与 EPOLLIN 或 EPOLLOUT 事件一起使用，以检测套接字是否处于错误状态。
   * 当 epoll 实例接收到 EPOLLERR 事件时，应用程序通常也需要检查 EPOLLIN 或 EPOLLOUT 事件，以确定套接字的状态。
  if (event.events & EPOLLERR) {
    PLOG_ERR("EPOLLERR", -1, "detect fd[%d] error, remove it, errno[%d] errmsg[%s]", fd, errno, strerror(errno));
    return IOLoop::Instance()->RemoveHandler(fd);
  }
   */

  if (event.events & EPOLLIN and event.events & EPOLLERR and errno != 0) {
    PLOG_ERR("in err", -1, "fd[%d] errno[%d] errmsg[%s]", fd, errno, strerror(errno));
    return IOLoop::Instance()->RemoveHandler(fd);
  }

  if (event.events & EPOLLIN) {
    return RecvData(fd);
  }

  if (event.events & EPOLLOUT and event.events & EPOLLERR and errno != 0) {
    PLOG_ERR("out err", -1, "fd[%d] errno[%d] errmsg[%s]", fd, errno, strerror(errno));
    return IOLoop::Instance()->RemoveHandler(fd);
  }

  if (event.events & EPOLLOUT) {
    return SendData(fd);
  }

  PLOG_ERR("error event", my_errorcode::kNotSupportEpollEvent, "events[%u] not support", static_cast<uint32_t>(event.events));
  return my_errorcode::kNotSupportEpollEvent;
}

ServerHandler::ServerHandler(uint16_t port)
    : port_(port) {
}

int ServerHandler::Init() {
  int fd;
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  int ret = socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
  // int ret = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (ret < 0) {
    PLOG_ERR("socket err", ret, "create socket err errno[%d] errmsg[%s]", errno, strerror(errno));
    return ret;
  }
  fd = ret;
  PLOG_RUN("create socket[%d] succ", fd);

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port_);

  ret = bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
  if (ret < 0) {
    PLOG_ERR("bind err", ret, "fail to bind port[%u] errno[%d] errmsg[%s]", port_, errno, strerror(errno));
    return ret;
  }
  PLOG_RUN("bind port[%d] succ", port_);

  constexpr int kMaxPending = 1024;
  ret = listen(fd, kMaxPending);
  if (ret < 0) {
    PLOG_ERR("listen err", ret, "fail to listen errno[%d] errmsg[%s]", errno, strerror(errno));
    return ret;
  }

  PLOG_RUN("listen fd[%d] succ", fd);

  return IOLoop::Instance()->AddHandler(fd, this, EPOLLIN);
}

int ServerHandler::Handle(struct epoll_event event) {
  PLOG_RUN("server handler accpet");
  const int fd = event.data.fd;
  struct sockaddr_in client_sockaddr;
  uint32_t client_sockaddr_length = sizeof(client_sockaddr);

  int ret = accept4(fd, reinterpret_cast<struct sockaddr*>(&client_sockaddr), &client_sockaddr_length,
          SOCK_NONBLOCK | SOCK_CLOEXEC);
  // int ret = accept4(fd, reinterpret_cast<struct sockaddr*>(&client_sockaddr), &client_sockaddr_length, SOCK_CLOEXEC);
  if (ret < 0) {
    PLOG_ERR("accept err", ret, "accpet error");
    return ret;
  }

  const char * client_addr = inet_ntoa(client_sockaddr.sin_addr);
  const uint16_t client_port = ntohs(client_sockaddr.sin_port);
  const int client_fd = ret;

  fd_to_clientaddr_map_[client_fd] = std::make_pair(client_addr, client_port);
  PLOG_RUN("accpet connected fd[%d] client_fd[%d] client_addr[%s] client_port[%u]", fd, client_fd, client_addr, client_port);

  Handler* client_handler = new EchoHandler();
  IOLoop::Instance()->AddHandler(client_fd, client_handler, EPOLLIN | EPOLLET);
  return 0;
}
