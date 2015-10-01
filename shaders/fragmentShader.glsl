#version 330 core

in vec2 UV;

uniform sampler2D textureSampler;

out vec3 color;

void main() {
  color = vec3(0.5f,0.5f,0.5f) * texture(textureSampler, UV).rgb;
}