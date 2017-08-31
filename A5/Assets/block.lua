rootnode = gr.node('lua');

stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0);
green = gr.material({0.1, 0.9, 0.1}, {0.0, 0.0, 0.0}, 0);
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20);
red = gr.material({0.7, 0.1, 0.1}, {0.0, 0.0, 0.0}, 0);

PLATFORM = {1, 1, 1};

function createPlatform(x, y, z)
  platform = gr.mesh('cube', 'platform-' .. tostring(x) .. ',' .. tostring(y));
  platform:set_material(stone);
  platform:scale(table.unpack(PLATFORM));
  platform:translate(x + 0.5, y + 0.5, z + 0.5);
  return platform;
end

p1 = createPlatform(0, 0, 0);

rootnode:add_child(p1);

return rootnode;
