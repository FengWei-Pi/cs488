#pragma once
#include "Platform.hpp"
#include <functional>
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include <string>

struct Level {
  std::map<unsigned int, double> platformTimes;
  std::vector<Platform> platforms;
  std::map<unsigned int, std::function<glm::vec3(float)>> platformUpdateVFns;
  static Level read(std::string filename);
};
