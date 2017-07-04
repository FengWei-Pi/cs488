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
  vec3 position_CameraSpace; // Eye-space position
  vec3 normal_CameraSpace;   // Eye-space normal
  LightSource light_CameraSpace;
  vec4 position_LightSpace; // Light space coordinates of current point
} vs_out;

uniform mat4 LightView;
uniform mat4 LightPerspective;

void main() {
  vec4 pos4 = vec4(position, 1.0);

  //-- Convert position and normal to Eye-Space:
  vs_out.position_CameraSpace = (View * Model * pos4).xyz;
  vs_out.normal_CameraSpace = normalize(transpose(inverse(mat3(View * Model))) * normal);
  vs_out.position_LightSpace = LightPerspective * LightView * Model * vec4(position, 1.0);

  // The light remains at a constant location in view space.
  LightSource light_CameraSpace;
  light_CameraSpace.rgbIntensity = light.rgbIntensity;
  light_CameraSpace.position =  light.position;

  vs_out.light_CameraSpace = light_CameraSpace;

  gl_Position = Perspective * View * Model * vec4(position, 1.0);
}
