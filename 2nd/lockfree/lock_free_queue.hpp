#pragma once

#include <cstdio>
#include <array>
#include <atomic>

namespace my_lock_free {
template<typename T, uint32_t length = 128>
class LockFreeQueue {
 public:
  LockFreeQueue() {
    ring_buffer_.resize(kRingBufferLength);
  }

  ~LockFreeQueue() {
    Clear();
    std::vector<T>().swap(ring_buffer_);
  }

  LockFreeQueue(const LockFreeQueue & other) = delete;
  LockFreeQueue operator=(const LockFreeQueue & other) = delete;

  LockFreeQueue(LockFreeQueue && other) = delete;
  LockFreeQueue operator=(LockFreeQueue && other) = delete;

  bool Push(const T & t) {
    const uint32_t old_write_pos = write_position_.load();
    const uint32_t new_write_pos = NextPosition(old_write_pos);
    const uint32_t read_pos = read_position_.load();

    if (new_write_pos == read_pos) {
      return false;
    }

    ring_buffer_[old_write_pos] = t;
    write_position_.store(new_write_pos);
    return true;
  }

  bool Pop(T & t) {
    if (Empty()) {
      return false;
    }

    const uint32_t old_read_pos = read_position_.load();
    const uint32_t new_read_pos = NextPosition(old_read_pos);
    t = ring_buffer_[old_read_pos];
    read_position_.store(new_read_pos);

    return true;
  }

  void Clear() {
    const uint32_t read_pos = read_position_.load();
    const uint32_t write_pos = write_position_.load();

    if (read_pos != write_pos) {
      read_position_.store(write_pos);
    }
  }

  uint32_t Size() {
    const uint32_t read_pos = read_position_.load();
    const uint32_t write_pos = write_position_.load();

    if (Empty()) {
      return 0;
    }

    if (write_pos < read_pos) {
      return kRingBufferLength - read_pos + write_pos;
    } else {
      return write_pos - read_pos;
    }
  }

  bool Empty() const noexcept {
    return read_position_.load() == write_position_.load();
  }

 private:
  uint32_t NextPosition(uint32_t position) {
    return position + 1 % kRingBufferLength;
  }

  static constexpr size_t kRingBufferLength = length + 1;
  std::vector<T> ring_buffer_;

  std::atomic<uint32_t> read_position_{0};
  std::atomic<uint32_t> write_position_{0};
};
}  // namespace my_lock_free
