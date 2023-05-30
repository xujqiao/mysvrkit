#pragma once

#include <cstdint>

#include <sys/epoll.h>
#include <unordered_map>

class Handler {
 public:
  virtual ~Handler() = default;
  virtual int Handle(struct epoll_event event) = 0;
};

/**
 * epoll 事件轮询
 */
class IOLoop {
public:
  static IOLoop *Instance();

  int Init();

  ~IOLoop();

  void Start();

  int AddHandler(int fd, Handler* handler, unsigned int events);

  int ModifyHandlerIn(int fd);

  int ModifyHandlerOut(int fd);

  int ModifyHandler(int fd, unsigned int events);

  int RemoveHandler(int fd);

private:
  IOLoop() = default;

  int epfd_;
  std::unordered_map<int, Handler*> handlers_;
};

class EchoHandler : public Handler {
public:
  EchoHandler() = default;

  int Handle(struct epoll_event event) override;

  int RecvData(int fd);

  int SendData(int fd);

private:
  bool has_received_ = false;
  std::string received_data_;
};

class ServerHandler : public Handler {
public:
  explicit ServerHandler(uint16_t port);

  int Init();

  int Handle(struct epoll_event event) override;

private:
  uint16_t port_;
  std::unordered_map<int, std::pair<const char *, uint16_t>> fd_to_clientaddr_map_;
};
