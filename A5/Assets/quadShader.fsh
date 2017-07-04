// #version 330 core
// out vec4 FragColor;
//
// in vec2 TexCoords;
//
// uniform sampler2D depthTexture;
// uniform float nearPlane;
// uniform float farPlane;
//
// // required when using a perspective projection matrix
// float LinearizeDepth(float depth) {
//   float z = depth * 2.0 - 1.0; // Back to NDC
//   return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
// }
//
// void main() {
//   float depthValue = texture(depthTexture, TexCoords).r;
//   // FragColor = vec4(vec3(LinearizeDepth(depthValue) / farPlane), 1.0); // perspective
//   FragColor = vec4(vec3(depthValue), 1.0); // orthographic
// }

#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D renderedTexture;
uniform sampler2D depthTexture;

void main(){
	color = vec4(texture( renderedTexture, UV ).xyz, 1);
	// color = vec4(vec3(texture(depthTexture, UV).r), 1);
}
