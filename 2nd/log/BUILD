cc_library(
  name = "mylog",
  srcs = [
    "log.h",
    "log.cpp",
  ],
  deps = [
    "//2nd/callgraph:mycallgraph",
    "//2nd/errcode:myerrcode",
    ":mylogfilemgr",
  ],
  copts = [
    "--std=c++17",
  ],
  visibility = ["//visibility:public"],
)

cc_library(
  name = "mylogfilemgr",
  srcs = [
    "log_file_mgr.h",
    "log_file_mgr.cpp",
  ],
  deps = [
    "//2nd/errcode:myerrcode",
  ],
  copts = [
    "--std=c++17",
  ],
  visibility = ["//visibility:public"],
)

cc_binary(
  name = "mylogtest",
  srcs = [
    "logtest.cpp",
  ],
  deps = [
    ":mylog",
  ],
  copts = [
    "--std=c++17",
  ],
  linkstatic = 1,
)

cc_binary(
  name = "mylogfilemgrtest",
  srcs = [
    "logfilemgrtest.cpp",
  ],
  deps = [
    ":mylogfilemgr",
  ],
  copts = [
    "--std=c++17",
  ],
  linkstatic = 1,
)


