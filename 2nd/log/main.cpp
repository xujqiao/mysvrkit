#include "log.h"

int main() {
  PLOG_INIT();
  CallGraphGenGraphId();

  PLOG_RUN("hello");
  PLOG_RUN("hello %d", 3);
  PLOG_ERR("hello", 2832123322, "re[%d]", 5);
  PLOG_ERR("hello", 2832123322, "re[%d] hello world[%s]", 5, "wer23");
  PLOG_ERR("ioloop init", -202, "init fail");
  return 0;
}
