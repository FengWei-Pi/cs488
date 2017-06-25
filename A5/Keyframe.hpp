#pragma once
#include <map>
#include <string>
#include <glm/glm.hpp>

struct Keyframe {
  std::map<std::string, double> rotations;
};
