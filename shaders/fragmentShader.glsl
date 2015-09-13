#version 330 core

// in vec3 fragmentColor;
in vec2 UV;

uniform sampler2D textureSampler;

out vec3 color;

void main() {
//  color = fragmentColor;
  color = texture(textureSampler, UV).rgb;
}