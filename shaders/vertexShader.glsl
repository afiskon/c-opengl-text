#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;

uniform mat4 MVP;

void main() {
  fragmentColor = vertexColor;

  vec4 temp = vec4(vertexPos, 1);
  gl_Position = MVP * temp;
}