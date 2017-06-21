-- A more macho version of simple_cows.py, in which cows aren't
-- spheres, they're cow-shaped polyhedral models.

-- We'll need an extra function that knows how to read Wavefront .OBJ
-- files.

stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
green = gr.material({0.1, 0.9, 0.1}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)
red = gr.material({0.7, 0.1, 0.1}, {0.0, 0.0, 0.0}, 0)

cylinder = gr.mesh('cylinder', 'cylinder.obj');

-- ##############################################
--
-- ##############################################

scene = gr.node('scene');

-- Grass
Grass = {30, 30, 30};
grass = gr.mesh('grass', 'plane.obj');
scene:add_child(grass);
grass:set_material(green)
grass:scale(30, 30, 30);

-- Carpet
Carpet = {2, 1, Grass[3]};
CarpetHeight = 0.1;
carpet = gr.mesh('carpet', 'plane.obj')
scene:add_child(carpet)
carpet:set_material(red)
carpet:scale(Carpet[1], Carpet[2], Carpet[3]);
carpet:translate(0, CarpetHeight, 0);

-- Pillar
PillarEnd = {0.6, 0.25, 0.6};
PillarColumn = {PillarEnd[1] * 0.9 / 2, 2.5, PillarEnd[3] * 0.9 / 2};

pillar = gr.node('pillar');
pillar:translate(0, CarpetHeight, 0);

pillarEnd = gr.mesh('pillarEnd', 'cube.obj');
pillarEnd:set_material(stone);
pillarEnd:translate(-0.5, 0, -0.5);
pillarEnd:scale(PillarEnd[1], PillarEnd[2], PillarEnd[3]);

pillarColumn = gr.mesh('centerPillar', 'cylinder.obj');
pillarColumn:set_material(stone);
pillarColumn:scale(PillarColumn[1], PillarColumn[2], PillarColumn[3]);

pillarEndTop = gr.node('pillarEndTop');
pillarEndTop:add_child(pillarEnd);
pillarEndTop:translate(0, PillarEnd[2] + PillarColumn[2], 0);
pillar:add_child(pillarEndTop);

centerPillar = gr.node('centerPillar')
centerPillar:add_child(pillarColumn);
centerPillar:translate(0, PillarEnd[2], 0);
pillar:add_child(centerPillar);

pillarEndBottom = gr.node('pillarEndBottom');
pillarEndBottom:add_child(pillarEnd);
pillar:add_child(pillarEndBottom);

-- Draw Pillars
for i = 1, 6 do
  pillarLeft = gr.node('pillar-left' .. tostring(i))
  pillarLeft:translate(-2, 0, 4 * i);
  pillarLeft:add_child(pillar);

  pillarRight = gr.node('pillar-right' .. tostring(i))
  pillarRight:translate(2, 0, 4 * i);
  pillarRight:add_child(pillar);

  scene:add_child(pillarLeft);
  scene:add_child(pillarRight);
end

-- #############################################
-- Read in the cow model from a separate file.
-- #############################################

cow_poly = gr.mesh('cow', 'cow.obj')
factor = 2.0/(2.76+3.637)

cow_poly:set_material(hide)

cow_poly:translate(0.0, 3.637, 0.0)
cow_poly:scale(factor, factor, factor)
cow_poly:translate(0.0, CarpetHeight, 0.0)

holyCow = gr.node('holyCow');
holyCow:add_child(cow_poly);
holyCow:rotate('Y', -90);
holyCow:translate(0, 0, 15);


scene:add_child(holyCow);


gr.render(scene,
	  'sample.png', 500, 500,
	  {0, 2, 30}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {gr.light({200, 202, 430}, {0.8, 0.8, 0.8}, {1, 0, 0})})
