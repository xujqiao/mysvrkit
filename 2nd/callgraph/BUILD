cc_library(
  name = "mycallgraph",
  srcs = [
    "call_graph.h",
    "call_graph.cpp",
  ],
  copts = [
    "--std=c++17",
  ],
  visibility = [
    "//visibility:public",
  ],
)

cc_binary(
  name = "mycallgraphtest",
  srcs = [
    "test.cpp",
  ],
  deps = [
    ":mycallgraph",
  ],
  copts = [
    "--std=c++17",
  ],
  linkstatic = 1,
)
