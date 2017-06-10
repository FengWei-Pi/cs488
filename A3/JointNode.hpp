#pragma once

#include "SceneNode.hpp"

class JointNode : public SceneNode {
public:
  JointNode(const std::string & name);
  virtual ~JointNode();

  void rotateAboutX(double x);
  void set_joint_x(double min, double init, double max);
  void rotateAboutY(double y);
  void set_joint_y(double min, double init, double max);

  struct JointRange {
    double min, init, max;
  };


  JointRange m_joint_x, m_joint_y;
};
