#include "Animation.hpp"
#include <cassert>
#include <iostream>

Animation::Animation(const double delta) : delta(delta) {

}

void Animation::push(Keyframe kf) {
  keyframes.push_back(kf);
}

Keyframe Animation::get(double t) {
  assert(keyframes.size() > 0);

  const int n = keyframes.size();
  const double time = std::fmod(t, delta * n);

  for (int i = 0, j = 1; i < n; i += 1, j = (j + 1) % n) {
    const Keyframe& I = keyframes.at(i);
    const Keyframe& J = keyframes.at(j);
    const double t = (time/delta - i);

    if (std::fabs(t) < 0.001) {
      return I;
    }

    if (t < 1.0) {
      Keyframe interpolation;
      for (std::pair<std::string, double> pair : I.rotations) {
        const std::string& jointName = pair.first;
        const double iRotation = pair.second;
        const double jRotation = J.rotations.at(jointName);

        interpolation.rotations[jointName] = (1 - t) * iRotation + t * jRotation;
      }

      for (std::pair<std::string, glm::vec3> pair: I.positions) {
        const std::string& jointName = pair.first;
        const glm::vec3& iPosition = pair.second;
        const glm::vec3& jPosition = J.positions.at(jointName);

        interpolation.positions[jointName] = float(1 - t) * iPosition + float(t) * jPosition;
      }

      return interpolation;
    }
  }

  return keyframes.back();
}
