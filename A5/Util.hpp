#pragma once

#include <glm/glm.hpp>

namespace Util {
  double mlerp(double from, double to, double t, double n);
  double mShortDist(double from, double to, double n);
  template <typename T> T normalize(T v);
}


template <typename T>
T Util::normalize(T v) {
 if (glm::length(v) > 0.00001) {
   return glm::normalize(v);
 }

 return v;
}
