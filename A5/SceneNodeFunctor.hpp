#pragma once

#include "GeometryNode.hpp"
#include "SceneNode.hpp"
#include "JointNode.hpp"

template <typename R, typename T>
class SceneNodeFunctor {
public:
  virtual R operator()(T& t, GeometryNode& node) = 0;
  virtual R operator()(T& t, SceneNode& node) = 0;
  virtual R operator()(T& t, JointNode& node) = 0;
};
