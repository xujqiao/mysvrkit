#pragma once

#include <string>
#include <cstdio>

class LogFileMgr {
 public:
  LogFileMgr() = default;
  virtual ~LogFileMgr();

  int Init(const char * log_file_path);

  int PrintLog(const std::string & log_string);
 private:
  int LogFileRotate();

  int RefreshFilePtr();

  std::string log_file_path_;
  std::string current_log_file_path_;
  FILE * ptr_log_file_;
};
