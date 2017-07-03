#version 330

struct LightSource {
  vec3 position;
  vec3 rgbIntensity;
};

in VsOutFsIn {
	vec3 position_ES; // Eye-space position
	vec3 normal_ES;   // Eye-space normal
  vec4 FragPosLightSpace; // light space position of coordinate
	LightSource light;
} fs_in;


out vec4 fragColour;

struct Material {
  vec3 kd;
  vec3 ks;
  float shininess;
};
uniform Material material;

// Ambient light intensity for each RGB component.
uniform vec3 ambientIntensity;

uniform sampler2D depthTexture;

float ShadowCalculation(vec4 fragPosLightSpace)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(depthTexture, projCoords.xy).r;
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  // check whether current frag pos is in shadow
  float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

  return shadow;
}

vec3 phongModel(vec3 fragPosition, vec3 fragNormal) {
	LightSource light = fs_in.light;

  // Direction from fragment to light source.
  vec3 l = normalize(light.position - fragPosition);

  // Direction from fragment to viewer (origin - fragPosition).
  vec3 v = normalize(-fragPosition.xyz);

  float n_dot_l = max(dot(fragNormal, l), 0.0);

	vec3 diffuse;
	diffuse = material.kd * n_dot_l;

  vec3 specular = vec3(0.0);

  if (n_dot_l > 0.0) {
		// Halfway vector.
		vec3 h = normalize(v + l);
    float n_dot_h = max(dot(fragNormal, h), 0.0);

    specular = material.ks * pow(n_dot_h, material.shininess);
  }

  float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
  return ambientIntensity + light.rgbIntensity * (specular + diffuse);
}

void main() {
	fragColour = vec4(phongModel(fs_in.position_ES, fs_in.normal_ES), 1.0);
}
