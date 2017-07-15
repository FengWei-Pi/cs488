#include "json.hpp"
#include "Level.hpp"
#include <fstream>
#include <streambuf>
#include <iostream>

#include "Util.hpp"

Level Level::read(std::string filename) {
  std::ifstream file(filename.c_str());
  std::string str;

  file.seekg(0, std::ios::end);
  str.reserve(file.tellg());
  file.seekg(0, std::ios::beg);

  str.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  nlohmann::json $json = nlohmann::json::parse(str);

  Level level;
  level.platforms.reserve($json["platforms"].size());

  for (auto& $platform : $json["platforms"]) {
    Platform platform{
      glm::vec3(
        $platform["position"][0].get<float>(),
        $platform["position"][1].get<float>(),
        $platform["position"][2].get<float>()
      ),
      glm::vec3(
        $platform["size"][0].get<float>(),
        $platform["size"][1].get<float>(),
        $platform["size"][2].get<float>()
      ),
      $platform["mass"].get<double>(),
      $platform["ttl"].get<double>()
    };

    level.platforms.push_back(platform);

    std::function<double(double)> sinusoid = Util::createSinusoid(
      $platform["updateVFn"]["A"].get<double>(),
      $platform["updateVFn"]["period"].get<double>(),
      $platform["updateVFn"]["k"].get<double>(),
      $platform["updateVFn"]["offset"].get<double>()
    );

    level.platformUpdateVFns[platform.getId()] = [sinusoid](double t) -> glm::vec3 {
      return float(sinusoid(t)) * glm::vec3(1, 0, 0);
    };

    level.platformTimes[platform.getId()] = 0;
  };

  return level;
}
