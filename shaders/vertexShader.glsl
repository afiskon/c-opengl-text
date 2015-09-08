#version 330 core

layout(location = 0) in vec3 vertexPos;
uniform mat4 MVP;

void main() {
  vec4 temp = vec4(vertexPos, 1);
  gl_Position = MVP * temp;
}