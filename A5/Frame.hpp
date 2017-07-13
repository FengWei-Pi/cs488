#pragma once
#include <map>
#include <string>
#include <glm/glm.hpp>

struct Frame {
  std::map<std::string, double> rotations;
  std::map<std::string, glm::vec3> positions;
};
