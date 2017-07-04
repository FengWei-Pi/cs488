#version 330

struct LightSource {
  vec3 position;
  vec3 rgbIntensity;
};

in VsOutFsIn {
  vec3 position_CameraSpace; // Eye-space position
  vec3 normal_CameraSpace;   // Eye-space normal
  vec4 position_LightSpace; // light space position of coordinate
  LightSource light_CameraSpace;
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

uniform sampler2DShadow depthTexture;

float ShadowCalculation(vec4 fragPosLightSpace) {
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform NDC [-1, 1] to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;

  vec3 lightDir = normalize(fs_in.light_CameraSpace.position - fs_in.position_CameraSpace);
  float bias = 0.005*tan(acos(dot(fs_in.normal_CameraSpace, lightDir)));

  bias = 0.00001 * clamp(bias, 0,0.01);

  return 1 - texture(depthTexture, vec3(projCoords.xy, projCoords.z - bias));
}

vec3 phongModel(vec3 fragPosition, vec3 fragNormal) {
  LightSource light = fs_in.light_CameraSpace;

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

  float shadow = ShadowCalculation(fs_in.position_LightSpace);
  return ambientIntensity + light.rgbIntensity * (1 - shadow) * (specular + diffuse);
}

void main() {
  fragColour = vec4(phongModel(fs_in.position_CameraSpace, fs_in.normal_CameraSpace), 1.0);
}
