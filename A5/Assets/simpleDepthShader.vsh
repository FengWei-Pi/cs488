// #version 330 core
// in vec3 position;
//
// uniform mat4 Perspective;
// uniform mat4 View;
// uniform mat4 Model;
//
// void main() {
//   gl_Position = Perspective * View * Model * vec4(position, 1.0);
// }


#version 330

// Model-Space coordinates
in vec3 position;
in vec3 normal;

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Perspective;


void main() {
	vec4 pos4 = vec4(position, 1.0);

  normal;

	gl_Position = Perspective * View * Model * vec4(position, 1.0);
}
