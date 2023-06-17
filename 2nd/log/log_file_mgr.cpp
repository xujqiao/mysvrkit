#include "log_file_mgr.h"

#include "2nd/errcode/errcode.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <iostream>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static std::string GetCurrentDay() {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  time_t now = tv.tv_sec;
  struct tm *local = localtime(&now);

  int year = local->tm_year + 1900;
  int month = local->tm_mon + 1;
  int day = local->tm_mday;

  char buffer[128] = {0};
  snprintf(buffer, sizeof(buffer), "%d%02d%02d", year, month, day);
  return buffer;
}


static int RotateLogFilePathWithDay(const std::string & log_file_path, std::string & new_log_file_path) {
  // find dot
  auto dot_position = log_file_path.rfind(".");
  bool dot_exist = dot_position != std::string::npos;
  if (dot_exist) {
    std::string log_file_path_before_dot = log_file_path.substr(0, dot_position);
    std::string log_file_path_after_dot = log_file_path.substr(dot_position);
    new_log_file_path = log_file_path_before_dot + GetCurrentDay() + log_file_path_after_dot;
    return 0;
  }

  new_log_file_path = log_file_path + GetCurrentDay();
  return 0;
}

static int RotateLogFilePathWithSize(const std::string & log_file_path) {
  struct stat log_file_stat;
  int ret = stat(log_file_path.c_str(), &log_file_stat);
  if (ret != 0) {
    return 0;
  }

  constexpr uint32_t kLogFileMaxSizeByte = 100 * 1024 * 1024;
  if (log_file_stat.st_size < kLogFileMaxSizeByte) {
    return 0;
  }

  constexpr uint32_t kLogFileMaxNum = 1000;
  for (uint32_t i = 0; i < kLogFileMaxNum; ++i) {
    std::string new_log_file_path = log_file_path + "." + std::to_string(i);
    struct stat new_log_file_stat;
    ret = stat(new_log_file_path.c_str(), &new_log_file_stat);
    if (ret == 0) {
      continue;
    }

    ret = rename(log_file_path.c_str(), new_log_file_path.c_str());
    if (ret == 0) {
      return 0;
    }
    return ret;
  }

  return 0;
}

LogFileMgr::~LogFileMgr() {
  if (ptr_log_file_ != nullptr) {
    fclose(ptr_log_file_);
    ptr_log_file_ = nullptr;
  }
}

int LogFileMgr::Init(const char * log_file_path) {
  log_file_path_ = std::string(log_file_path);
  current_log_file_path_ = log_file_path_;

  ptr_log_file_ = fopen(current_log_file_path_.c_str(), "a");

  return 0;
}

int LogFileMgr::LogFileRotate() {
  // time rotate
  std::string old_current_log_file_path = current_log_file_path_;
  int ret = RotateLogFilePathWithDay(log_file_path_, current_log_file_path_);
  if (ret != 0) {
    std::cerr << "RotateLogFilePathWithDay fail" << std::endl;
    return ret;
  }

  // size rotate
  ret = RotateLogFilePathWithSize(current_log_file_path_);
  if (ret != 0) {
    std::cerr << "RotateLogFilePathWithSize fail" << std::endl;
    return ret;
  }

  if (old_current_log_file_path != current_log_file_path_) {
    RefreshFilePtr();
  }

  return 0;
}

int LogFileMgr::RefreshFilePtr() {
  if (ptr_log_file_ != nullptr) {
    fclose(ptr_log_file_);
    ptr_log_file_ = nullptr;
  }

  ptr_log_file_ = fopen(current_log_file_path_.c_str(), "a");
  if (ptr_log_file_ == nullptr) {
    return Errcode::kLogFileOpenFail;
  }

  return 0;
}

int LogFileMgr::PrintLog(const std::string & log_string) {
  int ret = LogFileRotate();
  if (ret != 0) {
    return ret;
  }

  if (ptr_log_file_ == nullptr) {
    ptr_log_file_ = fopen(current_log_file_path_.c_str(), "a");
  }

  fprintf(ptr_log_file_, "%s\n", log_string.c_str());
  return 0;
}
