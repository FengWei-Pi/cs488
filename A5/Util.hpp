#pragma once

#include <glm/glm.hpp>
#include <functional>

namespace Util {
  double mlerp(double from, double to, double t, double n);
  double mShortDist(double from, double to, double n);
  template <typename T> T normalize(T v);
  std::function<glm::vec3(float)> createXAxisSinusoid(
    const float A,
    const float period,
    const float k,
    const float offset
  );
}


template <typename T>
T Util::normalize(T v) {
 if (glm::length(v) > 0.00001) {
   return glm::normalize(v);
 }

 return v;
}
