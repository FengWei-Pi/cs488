#version 330

in vec3 fragmentColor;

out vec4 fragColor;

void main() {
	fragColor = vec4( fragmentColor, 1 );
}
