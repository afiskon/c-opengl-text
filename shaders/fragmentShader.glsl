#version 330 core

in vec3 Norm;
in vec2 UV;

uniform sampler2D textureSampler;

out vec3 color;

void main() {
  color = vec3(1.0f,1.0f,1.0f) * texture(textureSampler, UV).rgb;
}