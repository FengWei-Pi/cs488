#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D picture;

void main(){
	color = vec4(texture( picture, UV ).xyz, 1);
}
