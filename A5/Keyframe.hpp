#pragma once

#include "Frame.hpp"

class Keyframe : public Frame {
  static int counter;
public:
  const int id;
  Keyframe();
};
