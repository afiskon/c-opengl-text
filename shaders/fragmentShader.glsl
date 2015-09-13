#version 330 core

in vec2 UV;

uniform sampler2D textureSampler;

out vec3 color;

void main() {
  color = texture(textureSampler, UV).rgb;
}