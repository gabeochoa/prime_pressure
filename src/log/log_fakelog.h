#pragma once

#include "log_level.h"
#include <iostream>
inline void log_me() { std::cerr << std::endl; }

template <typename T, typename... Args>
inline void log_me(T arg, Args... args) {
  std::cerr << arg << " ";
  log_me(args...);
}
#include "log_macros.h"
