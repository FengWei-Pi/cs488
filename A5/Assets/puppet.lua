-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
rootnode:scale( 0.5, 0.5, 0.5 )
rootnode:translate(0.0, 0.0, 0.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

LEG = {
  UPPER= {0.5, 2.5, 0.5},
  LOWER= {0.5, 2, 0.5},
  FOOT= {0.5, 0.5, 1.5}
}

ARM = {
  UPPER= {0.5, 2.5, 0.5},
  LOWER= {0.5, 2, 0.5},
  HAND= {0.5, 1, 0.5}
}

HEAD = {1, 1, 1}
NECK = {0.5, 0.5, 0.5}
TORSO = {2, 4, 0.5}

function createPuppet()
  body = gr.joint('body', {-180, 0, 180}, {-180, 0, 180});
  torso = gr.mesh('cube', 'torso');
  head = createHead();
  rightArm = createArm('right');
  leftArm = createArm('left');
  rightLeg = createLeg('right');
  leftLeg = createLeg('left');

  torso:set_material(white);

  body:add_child(torso);
  body:add_child(head);
  body:add_child(rightArm);
  body:add_child(leftArm);
  body:add_child(rightLeg);
  body:add_child(leftLeg);

  torso:translate(0, 0.5, 0);

  torso:scale(TORSO[1], TORSO[2], TORSO[3]);

  rightLeg:translate(-(TORSO[1] - LEG.UPPER[1])/2, 0, 0);
  leftLeg:translate((TORSO[1] - LEG.UPPER[1])/2, 0, 0);
  rightArm:translate(-(TORSO[1] + ARM.UPPER[1])/2, TORSO[2], 0);
  leftArm:translate((TORSO[1] + ARM.UPPER[1])/2, TORSO[2], 0);
  head:translate(0, TORSO[2], 0);

  return body;
end

function createHead()
  head = gr.mesh('cube', 'head');
  neck = gr.mesh('cube', 'neck');
  headJoint = gr.joint('head-joint', {-10, 0, 10}, {-10, 0, 10});
  neckJoint = gr.joint('neck-joint', {-30, 0, 30}, {-45, 0, 45});

  head:set_material(white);
  neck:set_material(green);

  neckJoint:add_child(headJoint);
  neckJoint:add_child(neck);
  headJoint:add_child(head);

  head:translate(0, 0.5, 0);
  neck:translate(0, 0.5, 0);

  head:scale(HEAD[1], HEAD[2], HEAD[3]);
  neck:scale(NECK[1], NECK[2], NECK[3]);

  headJoint:translate(0, NECK[2], 0);

  return neckJoint;
end

function createArm(name)
  upperArmJoint = gr.joint(name .. '-upper-arm-joint', {-180, 0, 90}, {-30, 0, 30});
  upperArm = gr.mesh('cube', name .. '-upper-arm');
  lowerArmJoint = gr.joint(name .. '-lower-arm-joint', {-145, 0, 0}, {-30, 0, 30});
  lowerArm = gr.mesh('cube', name .. '-lower-arm');
  handJoint = gr.joint(name .. '-hand-joint', {-30, 0, 30}, {-30, 0, 30});
  hand = gr.mesh('cube', name .. '-hand');

  upperArm:set_material(red);
  lowerArm:set_material(blue);
  hand:set_material(green);

  upperArmJoint:add_child(upperArm);
  upperArmJoint:add_child(lowerArmJoint);
  lowerArmJoint:add_child(lowerArm);
  lowerArmJoint:add_child(handJoint);
  handJoint:add_child(hand);

  upperArm:translate(0, -0.5, 0);
  lowerArm:translate(0, -0.5, 0);
  hand:translate(0, -0.5, 0);

  upperArm:scale(ARM.UPPER[1], ARM.UPPER[2], ARM.UPPER[3]);
  lowerArm:scale(ARM.LOWER[1], ARM.LOWER[2], ARM.LOWER[3]);
  hand:scale(ARM.HAND[1], ARM.HAND[2], ARM.HAND[3]);

  lowerArmJoint:translate(0, -ARM.UPPER[2], 0.0);
  handJoint:translate(0, -ARM.LOWER[2], 0);

  return upperArmJoint;
end

function createLeg(name)
  upperLegJoint = gr.joint(name .. '-upper-leg-joint', {-110, 0, 30}, {-30, 0, 30});
  upperLeg = gr.mesh('cube', name .. '-upper-leg');
  lowerLegJoint = gr.joint(name .. '-lower-leg-joint', {0, 0, 130}, {-30, 0, 30});
  lowerLeg = gr.mesh('cube', name .. '-lower-leg');
  footJoint = gr.joint(name .. '-foot-joint', {-15, 0, 25}, {-20, 0, 20});
  foot = gr.mesh('cube', name .. '-foot');

  upperLeg:set_material(red);
  lowerLeg:set_material(blue);
  foot:set_material(green);

  upperLegJoint:add_child(upperLeg);
  upperLegJoint:add_child(lowerLegJoint);
  lowerLegJoint:add_child(lowerLeg);
  lowerLegJoint:add_child(footJoint);
  footJoint:add_child(foot);

  upperLeg:translate(0, -0.5, 0);
  lowerLeg:translate(0, -0.5, 0);
  foot:translate(0, -0.5, 0);

  upperLeg:scale(LEG.UPPER[1], LEG.UPPER[2], LEG.UPPER[3]);
  lowerLeg:scale(LEG.LOWER[1], LEG.LOWER[2], LEG.LOWER[3]);
  foot:scale(LEG.FOOT[1], LEG.FOOT[2], LEG.FOOT[3]);

  lowerLegJoint:translate(0, -LEG.UPPER[2], 0.0);
  footJoint:translate(0, -LEG.LOWER[2], 0.0);

  foot:translate(0, 0, (LEG.FOOT[3] - LEG.LOWER[3]) / 2);

  return upperLegJoint;
end

rootnode:add_child(createPuppet())

return rootnode
