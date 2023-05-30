#include "server.h"
#include "log/log.h"

int main() {
  PLOG_INIT();
  CallGraphGenGraphId();
  int ret = IOLoop::Instance()->Init();
  if (ret != 0) {
    PLOG_ERR("ioloop init", ret, "init fail");
    return ret;
  }
  PLOG_RUN("ioloop init succ");

  ServerHandler server_handler(8877);
  ret = server_handler.Init();
  if (ret != 0) {
    PLOG_ERR("server init", ret, "init fail");
    return ret;
  }
  PLOG_RUN("server init succ");

  IOLoop::Instance()->Start();
  return 0;
}
