#include "Clock.hpp"

double Clock::getTime() {
  auto t_now = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::duration<double> >(t_now - t_start).count();
}
