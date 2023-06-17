#include "log.h"

#include <thread>
#include <chrono>

int main() {
  PLOG_INIT();
  CallGraphGenGraphId();

  PLOG_RUN("hello");
  PLOG_RUN("hello %d", 3);
  PLOG_ERR("hello", 2832123322, "re[%d]", 5);
  PLOG_ERR("hello", 2832123322, "re[%d] hello world[%s]", 5, "wer23");
  PLOG_ERR("ioloop init", -202, "init fail");

  std::this_thread::sleep_for(std::chrono::seconds(5));

  return 0;
}
