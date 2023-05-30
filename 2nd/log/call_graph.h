#pragma once

#include <string>

class MyCallGraph {
 public:
  static MyCallGraph * GetInstance();
  ~MyCallGraph() = default;

  void SetGraphId(const std::string & graph_id);
  void GenRandomGraphId();
  std::string GetGraphId() const;

 private:
  MyCallGraph() = default;

  std::string graph_id_;
};

#define CallGraphGenGraphId() \
  MyCallGraph::GetInstance()->GenRandomGraphId()
