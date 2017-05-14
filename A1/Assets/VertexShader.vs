#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
in vec3 position;
in vec4 vertexColor;

out vec4 fragmentColor;

void main() {
	gl_Position = P * V * M * vec4(position, 1.0);
  fragmentColor = vertexColor;
}
