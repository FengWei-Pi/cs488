#pragma once
#include "Frame.hpp"
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
  Keyframe back();
  std::tuple<Keyframe, Frame> get(double t);
  static Animation getPlayerWalkingAnimation();
  static Animation getPlayerStandingAnimation();
  static Animation getPlayerPreparingToJumpAnimation();
  static Animation getPlayerJumpingAnimation();
};
