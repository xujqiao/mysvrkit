#include <iostream>

#include "call_graph.h"

int main() {
  MyCallGraph::GetInstance()->GenRandomGraphId();
  std::cout << MyCallGraph::GetInstance()->GetGraphId() << std::endl;
  return 0;
}
