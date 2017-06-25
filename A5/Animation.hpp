#pragma once
#include "Keyframe.hpp"
#include <vector>

class Animation {
  std::vector<Keyframe> keyframes;
  const double delta;
public:
  Animation(const double delta);
  void push(Keyframe kf);
  Keyframe get(double t);
};
