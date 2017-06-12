#include "JointNode.hpp"
#include <cassert>

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
  : SceneNode(name)
{
  m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}

void JointNode::rotateAboutX(double diffX) {
  m_joint_x.v = glm::clamp(m_joint_x.v + diffX, m_joint_x.min, m_joint_x.max);
}

 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
  assert(min <= init && init <= max);
  m_joint_x.min = min;
  m_joint_x.init = init;
  m_joint_x.v = init;
  m_joint_x.oldV = init;
  m_joint_x.max = max;
}

void JointNode::rotateAboutY(double diffY) {
  m_joint_y.v = glm::clamp(m_joint_y.v + diffY, m_joint_y.min, m_joint_y.max);
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
  assert(min <= init && init <= max);
  m_joint_y.min = min;
  m_joint_y.init = init;
  m_joint_y.v = init;
  m_joint_y.oldV = init;
  m_joint_y.max = max;
}

void JointNode::reset() {
  m_joint_x.v = m_joint_x.init;
  m_joint_y.v = m_joint_y.init;
  save();
}

void JointNode::save() {
  m_joint_x.oldV = m_joint_x.v;
  m_joint_y.oldV = m_joint_y.v;
}
