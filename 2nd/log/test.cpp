#pragma once

#include <cstdio>
#include <iostream>
#include <chrono>
#include <bitset>
#include <cstdarg>
#include <cerrno>

// variadic template test
template<typename FirstArg>
void myprint(FirstArg first_arg) {
  std::cout << first_arg << std::endl;
}

template<typename FirstArg, typename... OtherArgs>
void myprint(FirstArg && first_arg, OtherArgs && ... other_args) {
  std::cout << first_arg << "|";
  myprint(std::forward<OtherArgs>(other_args)...);
}

// variadic template log
void mylog(const char * format, ...) {
  char mybuffer[1024];
  va_list args;
  va_start(args, format);
  vsnprintf(mybuffer, sizeof(mybuffer), format, args);
  va_end(args);
  std::cout << mybuffer << std::endl;
}

template<typename... Args>
void mylog2(const char * format, Args && ... args) {
  char mybuffer[1024];
  snprintf(mybuffer, sizeof(mybuffer)/sizeof(mybuffer[0]), format, std::forward<Args>(args)...);

  std::cout << mybuffer << std::endl;
}

int main() {
  // std::cout << getCurrentTime() << std::endl;

  myprint("hello", 1, std::bitset<8>(23));

  mylog("hello %d", 3);
  mylog2("hello %d", 3);

  // file test
  FILE * p_file = fopen("myfile.txt", "a");
  if (p_file == nullptr) {
    std::cerr << "open myfile.txt err " << strerror(errno) << std::endl;
    return -1;
  }

  fprintf(p_file, "hello world");

  fclose(p_file);

  return 0;
}
