#include "call_graph.h"

#include <chrono>
#include <random>

MyCallGraph * MyCallGraph::GetInstance() {
  static MyCallGraph call_graph;
  return &call_graph;
}

void MyCallGraph::SetGraphId(const std::string & graph_id) {
  graph_id_ = graph_id;
}

void MyCallGraph::GenRandomGraphId() {
  // ms timestamp
  auto ms_timestamp_generator = []() -> uint64_t {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());

    uint64_t now_timestamp_ms = now_ms.count();
    return now_timestamp_ms;
  };

  // random_string
  auto random_string_generator = [](uint32_t length) -> std::string {
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    for (uint32_t i = 0; i < length; ++i) {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
  };

  constexpr uint32_t kLength = 8;
  SetGraphId(random_string_generator(kLength) + std::to_string(ms_timestamp_generator()));
  return;
}

std::string MyCallGraph::GetGraphId() const {
  return graph_id_;
}

