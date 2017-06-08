-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
-- rootnode:rotate('y', -20.0)
rootnode:scale( 0.5, 0.5, 0.5 )
rootnode:translate(0.0, 0.0, -1.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

-- torso = gr.mesh('cube', 'torso')
-- rootnode:add_child(torso)
-- torso:set_material(white)
-- torso:scale(0.5,1.0,0.5);
--
-- head = gr.mesh('cube', 'head')
-- torso:add_child(head)
-- head:scale(1.0/0.5, 1.0, 1.0/0.5)
-- head:scale(0.4, 0.4, 0.4)
-- head:translate(0.0, 0.9, 0.0)
-- head:set_material(red)
--
-- neck = gr.mesh('sphere', 'neck')
-- torso:add_child(neck)
-- neck:scale(1.0/0.5, 1.0, 1.0/0.5)
-- neck:scale(0.15, 0.3, 0.15)
-- neck:translate(0.0, 0.6, 0.0)
-- neck:set_material(blue)
--
-- ears = gr.mesh('sphere', 'ears')
-- head:add_child(ears)
-- ears:scale(1.2, 0.08, 0.08)
-- ears:set_material(red)
-- ears:set_material(blue)
--
-- leftEye = gr.mesh('cube', 'leftEye')
-- head:add_child(leftEye)
-- leftEye:scale(0.2, 0.1, 0.1)
-- leftEye:translate(-0.2, 0.2, 0.5)
-- leftEye:set_material(blue)
--
-- rightEye = gr.mesh('cube', 'rightEye')
-- head:add_child(rightEye)
-- rightEye:scale(0.2, 0.1, 0.1)
-- rightEye:translate(0.2, 0.2, 0.5)
-- rightEye:set_material(blue)
--
-- leftShoulder = gr.mesh('sphere', 'leftShoulder')
-- torso:add_child(leftShoulder)
-- leftShoulder:scale(1/0.5,1.0,1/0.5);
-- leftShoulder:scale(0.2, 0.2, 0.2)
-- leftShoulder:translate(-0.4, 0.35, 0.0)
-- leftShoulder:set_material(blue)
--
-- leftArm = gr.mesh('cube', 'leftArm')
-- torso:add_child(leftArm)
-- leftArm:scale(1/0.5, 1.0, 1/0.5);
-- leftArm:scale(0.4, 0.1, 0.1)
-- leftArm:rotate('z', 50);
-- leftArm:translate(-0.8, 0.0, 0.0)
-- leftArm:set_material(red)
--
-- rightShoulder = gr.mesh('sphere', 'rightShoulder')
-- torso:add_child(rightShoulder)
-- rightShoulder:scale(1/0.5,1.0,1/0.5);
-- rightShoulder:scale(0.2, 0.2, 0.2)
-- rightShoulder:translate(0.4, 0.35, 0.0)
-- rightShoulder:set_material(blue)
--
-- rightArm = gr.mesh('cube', 'rightArm')
-- torso:add_child(rightArm)
-- rightArm:scale(1/0.5,1.0,1/0.5);
-- rightArm:scale(0.4, 0.1, 0.1)
-- rightArm:rotate('z', -50);
-- rightArm:translate(0.8, 0.0, 0.0)
-- rightArm:set_material(red)
--
-- leftHip = gr.mesh('sphere', 'leftHip')
-- torso:add_child(leftHip)
-- leftHip:scale(1/0.5,1.0,1/0.5);
-- leftHip:scale(0.21, 0.21, 0.21)
-- leftHip:translate(-0.38, -0.5, 0.0)
-- leftHip:set_material(blue)
--
-- rightHip = gr.mesh('sphere', 'rightHip')
-- torso:add_child(rightHip)
-- rightHip:scale(1/0.5,1.0,1/0.5);
-- rightHip:scale(0.21, 0.21, 0.21)
-- rightHip:translate(0.38, -0.5, 0.0)
-- rightHip:set_material(blue)
--
-- leftLeg = gr.mesh('cube', 'leftLeg')
-- leftHip:add_child(leftLeg)
-- leftLeg:scale(1/0.5, 1.0, 1/0.5);
-- leftLeg:scale(0.4, 0.1, 0.1)
-- leftLeg:translate(1,-2.8,0)
-- leftLeg:set_material(red)
--
-- rightLeg = gr.mesh('cube', 'rightLeg')
-- rightHip:add_child(rightLeg)
-- rightLeg:scale(1/0.5, 1.0, 1/0.5);
-- rightLeg:scale(0.4, 0.1, 0.1)
-- rightLeg:translate(-1,-2.8,0)
-- rightLeg:set_material(red)

function createPuppet()
  puppet = gr.node('puppet')

  head = createHead()
  body = createBody()

  bodyWidth = 2 + 0.5 + 0.5;
  headHeight = 1.5;
  headWidth = 1;

  body:translate(0, -headHeight, 0);

  puppet:add_child(body);
  puppet:add_child(head);

  head:translate((bodyWidth - headWidth)/2, 0, 0);

  return puppet;
end

function createBody()
  torso = gr.mesh('cube', 'torso');
  torso:translate(0.5, 0.5, 0.5);

  rightArm = createArm('right');
  leftArm = createArm('left');
  rightLeg = createLeg('right');
  leftLeg = createLeg('left');

  armWidth = 0.5;
  torsoWidth = 4 * armWidth;
  torsoHeight = 4.5;

  legWidth = 0.5;
  rightLegX = 0;
  leftLegX = torsoWidth - legWidth;

  torso:scale(torsoWidth, -torsoHeight, 0.5);
  torso:set_material(white);

  rightArm:translate(-armWidth, 0, 0);
  leftArm:translate(torsoWidth, 0, 0);
  rightLeg:translate(rightLegX, -torsoHeight, 0);
  leftLeg:translate(leftLegX, -torsoHeight, 0);

  body = gr.node('body');
  body:add_child(torso);
  body:add_child(rightArm);
  body:add_child(leftArm);
  body:add_child(rightLeg);
  body:add_child(leftLeg);

  body:translate(armWidth, 0, 0);

  return body;
end

function createHead()
  head = gr.mesh('cube', 'head');
  head:translate(0.5, 0.5, 0.5);

  head:set_material(white);
  headHeight = 1;
  headWidth = 1;
  headDepth = 1;

  head:scale(headWidth, headHeight, headDepth);

  neck = gr.mesh('cube', 'neck');
  neck:translate(0.5, 0.5, 0.5);

  neck:set_material(green);
  neckHeight = 0.5;
  neckWidth = 0.5;
  neckDepth = 0.5;

  neck:scale(neckWidth, neckHeight, neckDepth);
  neck:translate((headWidth - neckWidth) / 2, 0, 0);

  head:translate(0, neckHeight, 0);

  headJoint = gr.joint('headJoint', {-30, 0, 30}, {-80, 0, 80});
  headJoint:add_child(head);

  neckJoint = gr.joint('neckJoint', {-45, 0, 45}, {0, 0, 0});
  neckJoint:add_child(headJoint);
  neckJoint:add_child(neck);

  neckJoint:translate(0, -(headHeight + neckHeight), 0)

  return neckJoint;
end


function createArm(name)
  upperArmJoint = gr.joint(name .. '-upper-arm-joint', {-90, 0, 180}, {-30, 0, 30});
  upperArm = gr.mesh('cube', name .. '-upper-arm');
  lowerArmJoint = gr.joint(name .. '-lower-arm-joint', {0, 0, 145}, {-30, 0, 30});
  lowerArm = gr.mesh('cube', name .. '-lower-arm');
  handJoint = gr.joint(name .. '-hand-joint', {-30, 0, 30}, {-30, 0, 30});
  hand = gr.mesh('cube', name .. '-hand');

  upperArm:set_material(red);
  lowerArm:set_material(blue);
  hand:set_material(green);

  upperArm:translate(0.5, 0.5, 0.5);
  lowerArm:translate(0.5, 0.5, 0.5);
  hand:translate(0.5, 0.5, 0.5);

  upperArmJoint:add_child(upperArm);
  upperArmJoint:add_child(lowerArmJoint);
  lowerArmJoint:add_child(lowerArm);
  lowerArmJoint:add_child(handJoint);
  handJoint:add_child(hand);

  upperArmLength = 2.5;
  lowerArmLength = 2;
  handLength = 1;

  upperArm:scale(0.5, upperArmLength, 0.5);
  lowerArm:scale(0.5, lowerArmLength, 0.5);
  hand:scale(0.5, handLength, 0.5);

  lowerArmJoint:translate(0, upperArmLength, 0.0);
  handJoint:translate(0, lowerArmLength, 0);

  upperArmJoint:scale(1, -1, 1);
  return upperArmJoint;
end

function createLeg(name)
  upperLegJoint = gr.joint(name .. '-upper-leg-joint', {-30, 0, 110}, {-30, 0, 30});
  upperLeg = gr.mesh('cube', name .. '-upper-leg');
  lowerLegJoint = gr.joint(name .. '-lower-leg-joint', {-130, 0, 0}, {-30, 0, 30});
  lowerLeg = gr.mesh('cube', name .. '-lower-leg');
  footJoint = gr.joint(name .. '-foot-joint', {-20, 10, 20}, {-20, 0, 20});
  foot = gr.mesh('cube', name .. '-foot');

  upperLeg:set_material(red);
  lowerLeg:set_material(blue);
  foot:set_material(green);

  upperLeg:translate(0.5, 0.5, 0.5);
  lowerLeg:translate(0.5, 0.5, 0.5);
  foot:translate(0.5, 0.5, 0.5);

  upperLegJoint:add_child(upperLeg);
  upperLegJoint:add_child(lowerLegJoint);
  lowerLegJoint:add_child(lowerLeg);
  lowerLegJoint:add_child(footJoint);
  footJoint:add_child(foot);

  upperLegLength = 2.5;
  lowerLegLength = 2;
  footLength = 1.5;

  upperLeg:scale(0.5, upperLegLength, 0.5);
  lowerLeg:scale(0.5, lowerLegLength, 0.5);
  foot:scale(0.5, 0.5, footLength);

  lowerLegJoint:translate(0, upperLegLength, 0.0);
  footJoint:translate(0, lowerLegLength, 0);

  upperLegJoint:scale(1, -1, 1);

  return upperLegJoint;
end

rootnode:add_child(createPuppet())

return rootnode
