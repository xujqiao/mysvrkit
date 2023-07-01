#include <gtest/gtest.h>

#include <thread>

#include "2nd/lockfree/lock_free_queue.hpp"

// Demonstrate some basic assertions.
TEST(LockFreeQueueTest, BVT) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");

  my_lock_free::LockFreeQueue<int> my_queue;

  // push
  EXPECT_TRUE(my_queue.Push(3));
  EXPECT_TRUE(my_queue.Push(9));
  EXPECT_TRUE(my_queue.Push(19));
  // size
  EXPECT_EQ(static_cast<uint32_t>(3), my_queue.Size());

  // pop
  int element;
  EXPECT_TRUE(my_queue.Pop(element));
  EXPECT_EQ(3, element);
  EXPECT_TRUE(my_queue.Pop(element));
  EXPECT_EQ(9, element);
  EXPECT_EQ(static_cast<uint32_t>(1), my_queue.Size());
  // empty
  EXPECT_FALSE(my_queue.Empty());

  // clear
  my_queue.Clear();
  EXPECT_TRUE(my_queue.Empty());
}

my_lock_free::LockFreeQueue<int, 2> my_global_queue;

void producer(uint32_t max_loop) {
  for (uint32_t i = 0; i < max_loop;) {
    bool push_succ = my_global_queue.Push(i);
    if (push_succ) {
      ++i;
    }
  }
}

void consumer(uint32_t & total_length) {
  total_length = 0;
  while (not my_global_queue.Empty()) {
    int element;
    my_global_queue.Pop(element);
    total_length++;
  }
}

TEST(LockFreeQueueTest, MultiThread) {
  uint32_t loop1 = 4;
  uint32_t loop2 = 3;
  uint32_t loop_result = 0;
  std::thread t1(producer, loop1);
  std::thread t2(producer, loop2);
  std::thread t3(consumer, std::ref(loop_result));
  t1.join();
  t2.join();
  t3.join();

  EXPECT_EQ(loop1 + loop2, loop_result);
  my_global_queue.Clear();
  EXPECT_TRUE(my_global_queue.Empty());
}
