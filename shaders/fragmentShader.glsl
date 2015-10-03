#version 330 core

//in vec3 Norm;
in vec2 UV;
in vec3 normModel;

struct DirectionalLight {
  vec3 Color;
  vec3 Direction;
  float AmbientIntensity;
  float DiffuseIntensity;
};

uniform sampler2D textureSampler;
uniform vec3 lightPos;
uniform DirectionalLight directionalLight;

out vec4 color;

void main() {
  vec4 ambientColor = vec4(directionalLight.Color, 1) * directionalLight.AmbientIntensity;

  float diffuseFactor = clamp(dot(normalize(normModel), -directionalLight.Direction), 0, 1);
  vec4 diffuseColor = vec4(directionalLight.Color, 1) * directionalLight.DiffuseIntensity * diffuseFactor;

  color = texture(textureSampler, UV) * (ambientColor + diffuseColor);
}