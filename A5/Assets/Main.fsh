#version 330

struct LightSource {
  vec3 direction;
  vec3 rgbIntensity;
};

in VsOutFsIn {
	vec3 position_ES; // Eye-space position
	vec3 normal_ES;   // Eye-space normal
  vec4 position_LS; // light space position of coordinate
	LightSource light_ES;
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

// https://learnopengl.com/#!Advanced-Lighting/Shadows/Shadow-Mapping
float ShadowCalculation(vec4 fragPosLightSpace) {
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform NDC [-1, 1] to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;

  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;

  vec3 lightDir = -normalize(fs_in.light_ES.direction);
  float bias = 0.005*tan(acos(dot(fs_in.normal_ES, lightDir)));

  bias = clamp(bias, 0,0.01);

  // PCF for smoother shadows
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(depthTexture, 0);
  for(int x = -1; x <= 1; ++x)
  {
      for(int y = -1; y <= 1; ++y)
      {
          float pcfDepth = texture(depthTexture, projCoords.xy + vec2(x, y) * texelSize).r;
          shadow += currentDepth - 0.000001 * bias > pcfDepth ? 1.0 : 0.0;
      }
  }
  shadow /= 9.0;



  return shadow;
}

vec3 phongModel(vec3 fragPosition, vec3 fragNormal) {
	LightSource light = fs_in.light_ES;

  // Direction from fragment to light source.
  vec3 l = - normalize(light.direction);

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

  float shadow = ShadowCalculation(fs_in.position_LS);
  return ambientIntensity + light.rgbIntensity * (1 - shadow) * (specular + diffuse);
}

void main() {
	fragColour = vec4(phongModel(fs_in.position_ES, fs_in.normal_ES), 1.0);
}
