cc_library(
  name = "mycallgraph",
  srcs = [
    "call_graph.h",
    "call_graph.cpp",
  ],
  copts = [
    "--std=c++17",
  ],
)

cc_library(
  name = "mylog",
  srcs = [
    "errcode.h",
    "log.h",
    "log.cpp",
  ],
  deps = [
    ":mycallgraph",
  ],
  copts = [
    "--std=c++17",
  ],
  visibility = ["//visibility:public"],
)

cc_binary(
  name = "mylogtest",
  srcs = [
    "test.cpp",
  ],
  copts = [
    "--std=c++17",
  ],
  linkstatic = 1,
)

cc_binary(
  name = "mylogmain",
  srcs = [
    "main.cpp",
  ],
  deps = [
    ":mylog",
    ":mycallgraph",
  ],
  copts = [
    "--std=c++17",
  ],
  linkstatic = 1,
)
