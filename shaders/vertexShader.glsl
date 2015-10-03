#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexNorm;
layout(location = 2) in vec2 vertexUV;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 V;
uniform vec3 lightPos;

out vec3 Norm;
out vec2 UV;

void main() {
  UV = vertexUV;
  Norm = vertexNorm;
  gl_Position = MVP * vec4(vertexPos, 1);
}