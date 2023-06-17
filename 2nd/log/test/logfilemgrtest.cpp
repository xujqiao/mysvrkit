#include <gtest/gtest.h>

#include "2nd/log/log_file_mgr.h"

#include <string>

TEST(LogFileMgrTest, BVT) {
  std::string log_file_path = "/Users/camxu/Documents/my_workspace/2nd/log/mylog.txt";
  LogFileMgr log_file_mgr(log_file_path.c_str());

  int ret = log_file_mgr.LogFileRotate();

  EXPECT_EQ(0, ret);
}

