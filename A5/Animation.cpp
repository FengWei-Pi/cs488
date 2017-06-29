#include "Animation.hpp"
#include <cassert>
#include <iostream>

Animation::Animation(
  const double delta,
  const AnimationType type
) : delta(delta), type(type) {

}

void Animation::push(Keyframe kf) {
  keyframes.push_back(kf);
}

Keyframe Animation::get(double t) {
  assert(keyframes.size() > 0);

  const int n = keyframes.size();

  if (type == SingleRun && t >= delta * (n - 1)) {
    return keyframes.back();
  }

  const double time = std::fmod(t, delta * n);

  for (int i = 0, j = 1; i < n && j < n; i += 1, j = (j + 1) % n) {
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

Animation Animation::getPlayerWalkingAnimation(double delta) {
  double bodyXRotation = 20;
  float yDispScale = 0.5;
  Keyframe one;

  Animation playerWalkingAnimation{delta, Loop};

  one.rotations["body"] = bodyXRotation;
  one.positions["body"] = glm::vec3(0, 0, 0) * yDispScale;
  one.rotations["head-joint"] = 0;
  one.rotations["neck-joint"] = 0;

  one.rotations["left-upper-arm-joint"] = -20;
  one.rotations["left-lower-arm-joint"] = -100;
  one.rotations["left-hand-joint"] = 0;

  one.rotations["right-upper-arm-joint"] = 60;
  one.rotations["right-lower-arm-joint"] = -45;
  one.rotations["right-hand-joint"] = 0;

  one.rotations["left-upper-leg-joint"] = 0;
  one.rotations["left-lower-leg-joint"] = 90;
  one.rotations["left-foot-joint"] = 0;

  one.rotations["right-upper-leg-joint"] = -45;
  one.rotations["right-lower-leg-joint"] = 0;
  one.rotations["right-foot-joint"] = 0;

  playerWalkingAnimation.push(one);

  Keyframe two;
  two.rotations["body"] = bodyXRotation;
  two.positions["body"] = glm::vec3(0, -0.5, 0) * yDispScale;
  two.rotations["head-joint"] = 0;
  two.rotations["neck-joint"] = 0;

  two.rotations["left-upper-arm-joint"] = -20;
  two.rotations["left-lower-arm-joint"] = -90;
  two.rotations["left-hand-joint"] = 0;

  two.rotations["right-upper-arm-joint"] = 40;
  two.rotations["right-lower-arm-joint"] = -15;
  two.rotations["right-hand-joint"] = 0;

  two.rotations["left-upper-leg-joint"] = -10;
  two.rotations["left-lower-leg-joint"] = 90;
  two.rotations["left-foot-joint"] = 20;

  two.rotations["right-upper-leg-joint"] = -45;
  two.rotations["right-lower-leg-joint"] = 45;
  two.rotations["right-foot-joint"] = -15;

  playerWalkingAnimation.push(two);

  Keyframe three;
  three.rotations["body"] = bodyXRotation;
  three.positions["body"] = glm::vec3(0, -0.25, 0) * yDispScale;
  three.rotations["head-joint"] = 0;
  three.rotations["neck-joint"] = 0;

  three.rotations["left-upper-arm-joint"] = 0;
  three.rotations["left-lower-arm-joint"] = -30;
  three.rotations["left-hand-joint"] = 0;

  three.rotations["right-upper-arm-joint"] = 0;
  three.rotations["right-lower-arm-joint"] = -5;
  three.rotations["right-hand-joint"] = 0;

  three.rotations["left-upper-leg-joint"] = -30;
  three.rotations["left-lower-leg-joint"] = 60;
  three.rotations["left-foot-joint"] = 20;

  three.rotations["right-upper-leg-joint"] = -20;
  three.rotations["right-lower-leg-joint"] = 30;
  three.rotations["right-foot-joint"] = -15;

  playerWalkingAnimation.push(three);

  Keyframe four;
  four.rotations["body"] = bodyXRotation;
  four.positions["body"] = glm::vec3(0, 0, 0) * yDispScale;
  four.rotations["head-joint"] = 0;
  four.rotations["neck-joint"] = 0;

  four.rotations["left-upper-arm-joint"] = 15;
  four.rotations["left-lower-arm-joint"] = -20;
  four.rotations["left-hand-joint"] = 0;

  four.rotations["right-upper-arm-joint"] = 0;
  four.rotations["right-lower-arm-joint"] = -80;
  four.rotations["right-hand-joint"] = 0;

  four.rotations["left-upper-leg-joint"] = -90;
  four.rotations["left-lower-leg-joint"] = 90;
  four.rotations["left-foot-joint"] = 20;

  four.rotations["right-upper-leg-joint"] = -10;
  four.rotations["right-lower-leg-joint"] = 10;
  four.rotations["right-foot-joint"] = 0;

  playerWalkingAnimation.push(four);


  Keyframe five;
  five.rotations["body"] = bodyXRotation;
  five.positions["body"] = glm::vec3(0, 0.4, 0) * yDispScale;
  five.rotations["head-joint"] = 0;
  five.rotations["neck-joint"] = 0;

  five.rotations["left-upper-arm-joint"] = 30;
  five.rotations["left-lower-arm-joint"] = -30;
  five.rotations["left-hand-joint"] = 0;

  five.rotations["right-upper-arm-joint"] = 0;
  five.rotations["right-lower-arm-joint"] = -90;
  five.rotations["right-hand-joint"] = 0;

  five.rotations["left-upper-leg-joint"] = -90;
  five.rotations["left-lower-leg-joint"] = 70;
  five.rotations["left-foot-joint"] = 10;

  five.rotations["right-upper-leg-joint"] = 15;
  five.rotations["right-lower-leg-joint"] = 0;
  five.rotations["right-foot-joint"] = 20;

  playerWalkingAnimation.push(five);

  Keyframe six;
  six.rotations["body"] = bodyXRotation;
  six.positions["body"] = glm::vec3(0, 0.25, 0) * yDispScale;
  six.rotations["head-joint"] = 0;
  six.rotations["neck-joint"] = 0;

  six.rotations["left-upper-arm-joint"] = 45;
  six.rotations["left-lower-arm-joint"] = -30;
  six.rotations["left-hand-joint"] = 0;

  six.rotations["right-upper-arm-joint"] = -30;
  six.rotations["right-lower-arm-joint"] = -95;
  six.rotations["right-hand-joint"] = 0;

  six.rotations["left-upper-leg-joint"] = -90;
  six.rotations["left-lower-leg-joint"] = 60;
  six.rotations["left-foot-joint"] = -10;

  six.rotations["right-upper-leg-joint"] = 0;
  six.rotations["right-lower-leg-joint"] = 60;
  six.rotations["right-foot-joint"] = 15;

  playerWalkingAnimation.push(six);

  Keyframe seven;
  seven.rotations["body"] = bodyXRotation;
  seven.positions["body"] = glm::vec3(0, 0, 0) * yDispScale;
  seven.rotations["head-joint"] = 0;
  seven.rotations["neck-joint"] = 0;

  seven.rotations["left-upper-arm-joint"] = 60;
  seven.rotations["left-lower-arm-joint"] = -45;
  seven.rotations["left-hand-joint"] = 0;

  seven.rotations["right-upper-arm-joint"] = -20;
  seven.rotations["right-lower-arm-joint"] = -100;
  seven.rotations["right-hand-joint"] = 0;

  seven.rotations["left-upper-leg-joint"] = -45;
  seven.rotations["left-lower-leg-joint"] = 0;
  seven.rotations["left-foot-joint"] = 0;

  seven.rotations["right-upper-leg-joint"] = 0;
  seven.rotations["right-lower-leg-joint"] = 90;
  seven.rotations["right-foot-joint"] = 0;

  playerWalkingAnimation.push(seven);

  Keyframe eight;
  eight.rotations["body"] = bodyXRotation;
  eight.positions["body"] = glm::vec3(0, -0.5, 0) * yDispScale;
  eight.rotations["head-joint"] = 0;
  eight.rotations["neck-joint"] = 0;

  eight.rotations["left-upper-arm-joint"] = 40;
  eight.rotations["left-lower-arm-joint"] = -15;
  eight.rotations["left-hand-joint"] = 0;

  eight.rotations["right-upper-arm-joint"] = -20;
  eight.rotations["right-lower-arm-joint"] = -90;
  eight.rotations["right-hand-joint"] = 0;

  eight.rotations["left-upper-leg-joint"] = -45;
  eight.rotations["left-lower-leg-joint"] = 45;
  eight.rotations["left-foot-joint"] = -15;

  eight.rotations["right-upper-leg-joint"] = -10;
  eight.rotations["right-lower-leg-joint"] = 90;
  eight.rotations["right-foot-joint"] = 20;

  playerWalkingAnimation.push(eight);

  Keyframe nine;
  nine.rotations["body"] = bodyXRotation;
  nine.positions["body"] = glm::vec3(0, -0.25, 0) * yDispScale;
  nine.rotations["head-joint"] = 0;
  nine.rotations["neck-joint"] = 0;

  nine.rotations["left-upper-arm-joint"] = 0;
  nine.rotations["left-lower-arm-joint"] = -5;
  nine.rotations["left-hand-joint"] = 0;

  nine.rotations["right-upper-arm-joint"] = 0;
  nine.rotations["right-lower-arm-joint"] = -30;
  nine.rotations["right-hand-joint"] = 0;

  nine.rotations["left-upper-leg-joint"] = -20;
  nine.rotations["left-lower-leg-joint"] = 30;
  nine.rotations["left-foot-joint"] = -15;

  nine.rotations["right-upper-leg-joint"] = -30;
  nine.rotations["right-lower-leg-joint"] = 60;
  nine.rotations["right-foot-joint"] = 20;

  playerWalkingAnimation.push(nine);

  Keyframe ten;
  ten.rotations["body"] = bodyXRotation;
  ten.positions["body"] = glm::vec3(0, 0, 0) * yDispScale;
  ten.rotations["head-joint"] = 0;
  ten.rotations["neck-joint"] = 0;

  ten.rotations["right-upper-arm-joint"] = 15;
  ten.rotations["right-lower-arm-joint"] = -20;
  ten.rotations["right-hand-joint"] = 0;

  ten.rotations["left-upper-arm-joint"] = 0;
  ten.rotations["left-lower-arm-joint"] = -80;
  ten.rotations["left-hand-joint"] = 0;

  ten.rotations["right-upper-leg-joint"] = -90;
  ten.rotations["right-lower-leg-joint"] = 90;
  ten.rotations["right-foot-joint"] = 20;

  ten.rotations["left-upper-leg-joint"] = -10;
  ten.rotations["left-lower-leg-joint"] = 10;
  ten.rotations["left-foot-joint"] = 0;

  playerWalkingAnimation.push(ten);

  Keyframe eleven;
  eleven.rotations["body"] = bodyXRotation;
  eleven.positions["body"] = glm::vec3(0, 0.4, 0) * yDispScale;
  eleven.rotations["head-joint"] = 0;
  eleven.rotations["neck-joint"] = 0;

  eleven.rotations["left-upper-arm-joint"] = 0;
  eleven.rotations["left-lower-arm-joint"] = -90;
  eleven.rotations["left-hand-joint"] = 0;

  eleven.rotations["right-upper-arm-joint"] = 30;
  eleven.rotations["right-lower-arm-joint"] = -30;
  eleven.rotations["right-hand-joint"] = 0;

  eleven.rotations["left-upper-leg-joint"] = 15;
  eleven.rotations["left-lower-leg-joint"] = 0;
  eleven.rotations["left-foot-joint"] = 20;

  eleven.rotations["right-upper-leg-joint"] = -90;
  eleven.rotations["right-lower-leg-joint"] = 70;
  eleven.rotations["right-foot-joint"] = 10;

  playerWalkingAnimation.push(eleven);

  Keyframe twelve;
  twelve.rotations["body"] = bodyXRotation;
  twelve.positions["body"] = glm::vec3(0, 0.25, 0) * yDispScale;
  twelve.rotations["head-joint"] = 0;
  twelve.rotations["neck-joint"] = 0;

  twelve.rotations["left-upper-arm-joint"] = -30;
  twelve.rotations["left-lower-arm-joint"] = -95;
  twelve.rotations["left-hand-joint"] = 0;

  twelve.rotations["right-upper-arm-joint"] = 45;
  twelve.rotations["right-lower-arm-joint"] = -30;
  twelve.rotations["right-hand-joint"] = 0;

  twelve.rotations["left-upper-leg-joint"] = 0;
  twelve.rotations["left-lower-leg-joint"] = 60;
  twelve.rotations["left-foot-joint"] = 15;

  twelve.rotations["right-upper-leg-joint"] = -90;
  twelve.rotations["right-lower-leg-joint"] = 60;
  twelve.rotations["right-foot-joint"] = -10;

  playerWalkingAnimation.push(twelve);

  return playerWalkingAnimation;
}

Animation Animation::getPlayerStandingAnimation() {
  Animation playerStandingAnimation{1.0, SingleRun};

  Keyframe one;
  one.rotations["body"] = 0;
  one.positions["body"] = glm::vec3(0, 0, 0);
  one.rotations["head-joint"] = 0;
  one.rotations["neck-joint"] = 0;

  one.rotations["left-upper-arm-joint"] = 0;
  one.rotations["left-lower-arm-joint"] = 0;
  one.rotations["left-hand-joint"] = 0;

  one.rotations["right-upper-arm-joint"] = 0;
  one.rotations["right-lower-arm-joint"] = 0;
  one.rotations["right-hand-joint"] = 0;

  one.rotations["left-upper-leg-joint"] = 0;
  one.rotations["left-lower-leg-joint"] = 0;
  one.rotations["left-foot-joint"] = 0;

  one.rotations["right-upper-leg-joint"] = 0;
  one.rotations["right-lower-leg-joint"] = 0;
  one.rotations["right-foot-joint"] = 0;

  playerStandingAnimation.push(one);

  return playerStandingAnimation;
}
