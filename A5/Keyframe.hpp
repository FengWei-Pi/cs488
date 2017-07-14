#pragma once

#include "Frame.hpp"
#include <string>

class Keyframe : public Frame {
  static int counter;
public:
  const int id;
  const std::string sound;
  Keyframe();
  Keyframe(std::string sound);
};
