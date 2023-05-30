#include "log.h"

#include "errcode.h"

#include <cstdio>
#include <cerrno>
#include <cstring>

#include <sys/time.h>

MyLog * MyLog::GetInstance() {
  static MyLog my_log;
  return &my_log;
}

MyLog::~MyLog() {
  if (ptr_log_file_ != nullptr) {
    fclose(ptr_log_file_);
    ptr_log_file_ = nullptr;
  }
}

int MyLog::Init(uint32_t log_level,
    const char * log_file_path) {
  log_level_ = log_level;

  ptr_log_file_ = fopen(log_file_path, "a");
  if (ptr_log_file_ == nullptr) {
    printf("open log_file_path[%s] error[%s]", log_file_path, strerror(errno));
    return Errcode::kLogFileNotExist;
  }

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
