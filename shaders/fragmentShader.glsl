#version 330 core

in vec2 UV;
in vec3 normModel;
in vec3 vertexWorldPos;

struct DirectionalLight {
  vec3 Color;
  vec3 Direction;
  float AmbientIntensity;
  float DiffuseIntensity;
};

uniform sampler2D textureSampler;
uniform vec3 cameraPos;
uniform float materialSpecularFactor;
uniform float materialSpecularIntensity;
uniform DirectionalLight directionalLight;

out vec4 color;

void main() {
  vec4 ambientColor = vec4(directionalLight.Color, 1) * directionalLight.AmbientIntensity;

  vec3 n = normalize(normModel);
  float diffuseFactor = clamp(dot(n, -directionalLight.Direction), 0, 1);
  vec4 diffuseColor = vec4(directionalLight.Color, 1) * directionalLight.DiffuseIntensity * diffuseFactor;

  vec3 vertexToCamera = normalize(cameraPos - vertexWorldPos);
  vec3 lightReflect = normalize(reflect(directionalLight.Direction, n));
  float specularFactor = pow(clamp(dot(vertexToCamera, lightReflect), 0, 1), materialSpecularFactor);
  vec4 specularColor = vec4(directionalLight.Color, 1) * materialSpecularIntensity * specularFactor;

  color = texture(textureSampler, UV) * (ambientColor + diffuseColor + specularColor);
}