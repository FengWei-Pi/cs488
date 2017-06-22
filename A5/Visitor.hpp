#pragma once

class SceneNode;
class GeometryNode;
class JointNode;

class Visitor {
public:
  virtual void visit(SceneNode& node) = 0;
  virtual void visit(GeometryNode& node) = 0;
  virtual void visit(JointNode& node) = 0;
};
