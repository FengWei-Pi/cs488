#version 330 core
in vec3 position;
in vec3 normal;

uniform mat4 LightPerspective;
uniform mat4 LightView;
uniform mat4 Model;

void main() {
  gl_Position = LightPerspective * LightView * Model * vec4(position, 1.0);
}
