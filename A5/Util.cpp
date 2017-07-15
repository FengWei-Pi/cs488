#include <cmath>
#include "Util.hpp"

/**
 * Algorithm borrowed from:
 * - https://gist.github.com/shaunlebron/8832585
 */

double Util::mShortDist(double a0, double a1, double max) {
  double da = std::fmod(a1 - a0, max);
  return std::fmod(2*da, max) - da;
}

double Util::mlerp(double a0, double a1, double t, double max) {
  return a0 + Util::mShortDist(a0, a1, max)*t;
}

std::function<glm::vec3(float)> Util::createXAxisSinusoid(
  const float A,
  const float period,
  const float k,
  const float offset
) {
  const float PI = glm::radians(180.0f);
  return [PI, A, k, period, offset](float t) -> glm::vec3 {
    return (A * std::sin(2 * PI / period * (t - k)) + offset) * glm::vec3(1, 0, 0);
  };
}
