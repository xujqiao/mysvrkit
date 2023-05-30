load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
  name = "myprotobuftestservice_pb",
  srcs = ["myprotobuftestservice.proto"],
)

cc_proto_library(
  name = "myprotobuftestservice_cc_pb",
  deps = [":myprotobuftestservice_pb"],
)

cc_binary(
  name = "myprotobuftestservice",
  srcs = ["main.cpp"],
  deps = [
    ":myprotobuftestservice_cc_pb",
  ],
)
