#version 330 core

in vec3 Norm;
in vec2 UV;
// in vec3 vertexWorldPos;

struct DirectionalLight {
  vec3 Color;
  float AmbientIntensity;
};

uniform sampler2D textureSampler;
uniform vec3 lightPos;
uniform DirectionalLight directionalLight;

out vec4 color;

void main() {
  color = texture(textureSampler, UV) * vec4(directionalLight.Color, 1) * directionalLight.AmbientIntensity;
}