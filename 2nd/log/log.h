#pragma once

#include "2nd/callgraph/call_graph.h"
#include "log_file_mgr.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <queue>
#include <thread>

enum LogLevel {
  kUnknownLogLevel = 0,
  kDebugLogLevel = 1,
  kRunLogLevel = 2,
  kErrorLogLevel = 3,
};

class MyLog {
 public:
  ~MyLog() = default;

  static MyLog * GetInstance();

  int Init(uint32_t log_level = LogLevel::kDebugLogLevel,
      const char * log_file_path = "/Users/camxu/Documents/my_workspace/2nd/log/mylog.txt");

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
    log_queue_.push(buffer);
    std::cout << log_queue_.size() << std::endl;
    return 0;
  }

  void LoopPrint();

 private:
  MyLog() = default;

  std::string GetCurrentTime();

  uint32_t log_level_;
  LogFileMgr log_file_mgr_;

  std::thread loop_print_thread_;
  std::queue<std::string> log_queue_;
};

#define PLOG_INIT() \
  MyLog::GetInstance()->Init()

#define PLOG_RUN(format, args...) \
  MyLog::GetInstance()->Log(LogLevel::kRunLogLevel, "%s:%d|" format, __FILE__, __LINE__, ##args)

#define PLOG_ERR(logic, ret, format, args...) \
  MyLog::GetInstance()->Log(LogLevel::kErrorLogLevel,\
      "%s:%d|%s|ret=%X(%d)|" format, __FILE__, __LINE__, logic, ret, ret, ##args)
