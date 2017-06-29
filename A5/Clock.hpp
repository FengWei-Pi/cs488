#pragma once

#include <chrono>

static std::chrono::high_resolution_clock::time_point t_start;

namespace Clock {
  double getTime();
}
