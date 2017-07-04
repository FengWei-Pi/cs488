#version 330

// Model-Space coordinates
in vec3 position;
in vec3 normal;

struct LightSource {
  vec3 direction;
  vec3 rgbIntensity;
};
uniform LightSource light;

uniform mat4 View;
uniform mat4 Model;
uniform mat4 Perspective;

out VsOutFsIn {
	vec3 position_ES; // Eye-space position
	vec3 normal_ES;   // Eye-space normal
	LightSource light_ES;
  vec4 position_LS; // Light space coordinates of current point
} vs_out;

uniform mat4 LightView;
uniform mat4 LightPerspective;

void main() {
	vec4 pos4 = vec4(position, 1.0);

	//-- Convert position and normal to Eye-Space:
	vs_out.position_ES = (View * Model * pos4).xyz;
	vs_out.normal_ES = normalize(transpose(inverse(mat3(View * Model))) * normal);
  vs_out.position_LS = LightPerspective * LightView * Model * vec4(position, 1.0);

  LightSource light_ES;
  light_ES.rgbIntensity = light.rgbIntensity;
  light_ES.direction =  (View * vec4(light.direction, 0)).xyz;

  vs_out.light_ES = light_ES;

	gl_Position = Perspective * View * Model * vec4(position, 1.0);
}
