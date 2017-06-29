#pragma once
#include "Keyframe.hpp"
#include <vector>

class Animation {
public:
  enum AnimationType {
    SingleRun,
    Loop
  };
private:
  std::vector<Keyframe> keyframes;
  const double delta;
  const AnimationType type;
public:
  Animation(const double delta, const AnimationType type);
  void push(Keyframe kf);
  Keyframe get(double t);
  static Animation getPlayerWalkingAnimation(double t);
  static Animation getPlayerStandingAnimation();
};
