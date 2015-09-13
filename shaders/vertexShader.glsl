#version 330 core

layout(location = 0) in vec3 vertexPos;
//layout(location = 1) in vec3 vertexColor;
layout(location = 1) in vec2 vertexUV;

out vec2 UV;
//out vec3 fragmentColor;

uniform mat4 MVP;

void main() {
//  fragmentColor = vertexColor;
  UV = vertexUV;

  gl_Position = MVP * vec4(vertexPos, 1);
}