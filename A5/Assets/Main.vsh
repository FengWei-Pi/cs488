#version 330

// Model-Space coordinates
in vec3 position;
in vec3 normal;

struct LightSource {
  vec3 position;
  vec3 rgbIntensity;
};
uniform LightSource light;

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Perspective;

out VsOutFsIn {
	vec3 position_ES; // Eye-space position
	vec3 normal_ES;   // Eye-space normal
  vec4 FragPosLightSpace; // Light space coordinates of current point
	LightSource light;
} vs_out;

uniform mat4 LightView;
uniform mat4 LightPerspective;

void main() {
	vec4 pos4 = vec4(position, 1.0);

	//-- Convert position and normal to Eye-Space:
	vs_out.position_ES = (View * Model * pos4).xyz;
	vs_out.normal_ES = normalize(transpose(inverse(mat3(View * Model))) * normal);
  vs_out.FragPosLightSpace = LightPerspective * LightView * Model * vec4(position, 1.0);
	vs_out.light = light;

	gl_Position = Perspective * View * Model * vec4(position, 1.0);
}
