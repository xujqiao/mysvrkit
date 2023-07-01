cc_test(
  name = "mylockfreequeuetest",
  size = "small",
  srcs = ["lock_free_queue_test.cpp"],
  deps = [
    "//3rd/gtest:gtest_main",
    "//2nd/lockfree:mylockfreequeue",
  ],
  copts = [
    "-std=c++17",
  ],
  linkopts = [
    "-std=c++17",
  ],
)
