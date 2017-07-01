#version 330 core
in vec3 position;

uniform mat4 Perspective;
uniform mat4 ModelView;

void main() {
    gl_Position = Perspective * ModelView * vec4(position, 1.0);
}