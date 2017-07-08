#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class AnimationTransformationReducer : public SceneNodeFunctor<glm::mat4, glm::mat4> {
  Keyframe& frame;
public:
  AnimationTransformationReducer(Keyframe& frame) : frame(frame) {}

  glm::mat4 operator()(glm::mat4& m, GeometryNode& node) {
    return m * node.trans;
  }

  glm::mat4 operator()(glm::mat4& m, SceneNode& node) {
    return m * node.trans;
  }

  glm::mat4 operator()(glm::mat4& m, JointNode& node) {
    glm::mat4 M = m * node.trans;

    float xAnimationRotation = 0;
    if (frame.rotations.find(node.m_name) != frame.rotations.end()) {
      xAnimationRotation = frame.rotations.at(node.m_name);
    }

    float xRotation = glm::radians(
      glm::clamp(
        node.m_joint_x.init + xAnimationRotation,
        node.m_joint_x.min,
        node.m_joint_x.max
      )
    );

    float yRotation = glm::radians(
      glm::clamp(
        node.m_joint_y.init,
        node.m_joint_y.min,
        node.m_joint_y.max
      )
    );

    M = M * glm::rotate(glm::mat4(), yRotation, glm::vec3(0, 1, 0));
    M = M * glm::rotate(glm::mat4(), xRotation, glm::vec3(1, 0, 0));

    if (frame.positions.find(node.m_name) != frame.positions.end()) {
      M = M * glm::translate(glm::mat4(), frame.positions.at(node.m_name));
    }

    return M;
  }
};
