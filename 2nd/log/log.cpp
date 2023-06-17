#include "log.h"

#include "2nd/errcode/errcode.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <chrono>

#include <sys/time.h>

MyLog * MyLog::GetInstance() {
  static MyLog my_log;
  return &my_log;
}

int MyLog::Init(uint32_t log_level,
    const char * log_file_path) {
  log_level_ = log_level;

  log_file_mgr_.Init(log_file_path);

  loop_print_thread_ = std::thread(&MyLog::LoopPrint, this);
  loop_print_thread_.detach();

  return 0;
}

std::string MyLog::GetCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time_t now = tv.tv_sec;
  struct tm *local = localtime(&now);

  int year = local->tm_year + 1900;
  int month = local->tm_mon + 1;
  int day = local->tm_mday;
  int hour = local->tm_hour;
  int minute = local->tm_min;
  int second = local->tm_sec;

  char buffer[128] = {0};
  snprintf(buffer, sizeof(buffer), "%d-%02d-%02d %02d:%02d:%02d.%d",
          year, month, day, hour, minute, second, tv.tv_usec);
  return buffer;
}

void MyLog::LoopPrint() {
  while (true) {
    if (log_queue_.empty()) {
      std::this_thread::yield();
      continue;
    }

    const std::string log_string = log_queue_.front();
    int ret = log_file_mgr_.PrintLog(log_queue_.front());
    if (ret != 0) {
      printf("err log file mgr print log ret[%d]", ret);
      continue;
    }
    log_queue_.pop();
  }
}
