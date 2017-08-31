#pragma once

#include "Visitor.hpp"
#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"
#include "SceneNodeFunctor.hpp"

#include <stack>

class TransformationCollector : public Visitor {
  std::stack<glm::mat4> transforms;
  void visitChildren(std::list<SceneNode*>& children) {
    for (SceneNode * child : children) {
      child->accept(*this);
    }
  }
  SceneNodeFunctor<glm::mat4, glm::mat4>& reduce;
  SceneNodeFunctor<void, glm::mat4>& sideEffect;
public:
  TransformationCollector(
    SceneNodeFunctor<glm::mat4, glm::mat4>& reduce,
    SceneNodeFunctor<void, glm::mat4>& sideEffect,
    glm::mat4 T
  ) : reduce(reduce), sideEffect(sideEffect) {
    transforms.push(T);
  }

  void visit(SceneNode& node) {
    glm::mat4 M = reduce(transforms.top(), node);
    sideEffect(M, node);
    transforms.push(M);
    visitChildren(node.children);
    transforms.pop();
  }

  void visit(GeometryNode& node) {
    glm::mat4 M = reduce(transforms.top(), node);
    sideEffect(M, node);
    transforms.push(M);
    visitChildren(node.children);
    transforms.pop();
  }

  void visit(JointNode& node) {
    glm::mat4 M = reduce(transforms.top(), node);
    sideEffect(M, node);
    transforms.push(M);
    visitChildren(node.children);
    transforms.pop();
  }
};
