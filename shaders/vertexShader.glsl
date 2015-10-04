#version 330 core

layout(location = 0) in vec3 vertexPos;
layout(location = 1) in vec3 vertexNorm;
layout(location = 2) in vec2 vertexUV;

uniform mat4 MVP;
uniform mat4 M;

out vec2 UV;
out vec3 normModel;
out vec3 vertexWorldPos;

void main() {
  UV = vertexUV;
  gl_Position = MVP * vec4(vertexPos, 1);

  normModel = (M * vec4(vertexNorm, 0)).xyz;
  vertexWorldPos = (M * vec4(vertexPos, 1)).xyz;
}