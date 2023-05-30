#include <iostream>

#include "gen_code/myprotobuftestservice.pb.h"

int main() {
  SearchReq req;
  req.set_query("hello");
  req.set_page_number(1);
  req.set_result_per_page(10);

  std::cout << req.ShortDebugString() << std::endl;
  return 0;
}
