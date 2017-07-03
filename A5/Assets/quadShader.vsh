// #version 330 core
// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 aTexCoords;
//
// out vec2 TexCoords;
//
// void main() {
//   TexCoords = aTexCoords;
//   gl_Position = vec4(aPos, 1.0);
// }


#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 vertexPosition_modelspace;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// // Values that stay constant for the whole mesh.
// uniform mat4 MVP;

void main(){

    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  vec4(vertexPosition_modelspace,1);

    // UV of the vertex. No special space for this one.
    UV = (vertexPosition_modelspace.xy + vec2(1, 1)) / 2;
}
