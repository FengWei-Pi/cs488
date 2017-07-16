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
  vec2 uv;
} fs_in;


out vec4 fragColour;

struct Material {
  vec3 kd;
  vec3 ks;
  float shininess;
};
uniform Material material;

uniform float alpha;

// Ambient light intensity for each RGB component.
uniform vec3 ambientIntensity;

uniform sampler2DShadow depthTexture;
uniform sampler2D picture;

vec2 poissonDisk[4] = vec2[](
   vec2( -0.94201624, -0.39906216 ),
   vec2( 0.94558609, -0.76890725 ),
   vec2( -0.094184101, -0.92938870 ),
   vec2( 0.34495938, 0.29387760 )
);

uniform int showTextures;
uniform int showShadows;

float ShadowCalculation(vec4 fragPosLightSpace) {
  if (showShadows == 0) {
    return 0;
  }

  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

// transform NDC [-1, 1] to [0,1] range
  projCoords = fragPosLightSpace.xyz * 0.5 + 0.5;

  if(projCoords.z > 1.0) {
    return 0;
  }

  vec3 lightDir = normalize(fs_in.light_CameraSpace.position - fs_in.position_CameraSpace);
  float bias = 0.005*tan(acos(clamp(dot(fs_in.normal_CameraSpace, lightDir), 0, 1)));

  bias = 0.00001 * clamp(bias, 0,0.01);

  /**
   * Poisson disk algorithm taken from
   * http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
   */

  float visibility=1.0;

  // Sample the shadow map 4 times
  for (int i=0;i<4;i++){
    int index = i;
    visibility -= 0.2 * (1.0- texture(depthTexture, vec3(projCoords.xy + poissonDisk[index]/700.0, projCoords.z - bias)));
  }

  return 1 - visibility;
}

vec3 phongModel(vec3 fragPosition, vec3 fragNormal) {
  LightSource light = fs_in.light_CameraSpace;

  // Direction from fragment to light source.
  vec3 l = normalize(light.position - fragPosition);

  // Direction from fragment to viewer (origin - fragPosition).
  vec3 v = normalize(-fragPosition.xyz);

  float n_dot_l = clamp(dot(fragNormal, l), 0, 1);

  vec3 diffuse;
  diffuse = material.kd * n_dot_l;

  vec3 specular = vec3(0.0);

  if (n_dot_l > 0.0) {
    // Halfway vector.
    vec3 h = normalize(v + l);
    float n_dot_h = clamp(dot(fragNormal, h), 0, 1);

    specular = material.ks * pow(n_dot_h, material.shininess);
  }

  float shadow = ShadowCalculation(fs_in.position_LightSpace);
  return ambientIntensity + light.rgbIntensity * (1 - shadow) * (specular + diffuse);
}

void main() {
  vec3 lighting = phongModel(fs_in.position_CameraSpace, fs_in.normal_CameraSpace);
  vec3 texCoord = texture(picture, fs_in.uv).rgb;

  if (showTextures == 0) {
    texCoord = vec3(1, 1, 1);
  }

  fragColour = vec4(lighting * texCoord, alpha);
}
