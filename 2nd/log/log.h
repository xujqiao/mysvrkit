#pragma once

#include "call_graph.h"

#include <cstdio>
#include <iostream>
#include <string>

enum LogLevel {
  kUnknownLogLevel = 0,
  kDebugLogLevel = 1,
  kRunLogLevel = 2,
  kErrorLogLevel = 3,
};

class MyLog {
 public:
  ~MyLog();

  static MyLog * GetInstance();

  int Init(uint32_t log_level = LogLevel::kDebugLogLevel,
      const char * log_file_path = "mylog.txt");

  template<typename... Args>
  int Log(uint32_t log_level, const std::string format, Args ... args) {
    if (log_level < log_level_) {
      return 0;
    }

    const char * sz_log_level = nullptr;
    switch (log_level) {
      case LogLevel::kDebugLogLevel:
        sz_log_level = "debug";
        break;
      case LogLevel::kRunLogLevel:
        sz_log_level = "run";
        break;
      case LogLevel::kErrorLogLevel:
        sz_log_level = "error";
        break;
      default:
        sz_log_level = "unknown";
        break;
    }

    char buffer[1024] = {0};
    snprintf(buffer, sizeof(buffer), (std::string("%s|") + std::string("%s|") + std::string("%s|") + format).c_str(),
        GetCurrentTime().c_str(),
        MyCallGraph::GetInstance()->GetGraphId().c_str(),
        sz_log_level,
        args...);
    std::cout << buffer << std::endl;
    // fprintf(ptr_log_file_, "%s\n", buffer);
    return 0;
  }

 private:
  MyLog() = default;

  std::string GetCurrentTime();

  uint32_t log_level_;
  FILE * ptr_log_file_;
};

#define PLOG_INIT() \
  MyLog::GetInstance()->Init()

#define PLOG_RUN(format, args...) \
  MyLog::GetInstance()->Log(LogLevel::kRunLogLevel, "%s:%d|" format, __FILE__, __LINE__, ##args)

#define PLOG_ERR(logic, ret, format, args...) \
  MyLog::GetInstance()->Log(LogLevel::kErrorLogLevel,\
      "%s:%d|%s|ret=%X(%d)|" format, __FILE__, __LINE__, logic, ret, ret, ##args)
