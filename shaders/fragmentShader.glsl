#version 330 core

in vec2 fragmentUV;
in vec3 fragmentNormal;
in vec3 fragmentPos;

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
  vec3 normal = normalize(fragmentNormal); // normal should be corrected after interpolation

  vec4 ambientColor = vec4(directionalLight.Color, 1) * directionalLight.AmbientIntensity;

  float diffuseFactor = clamp(dot(normal, -directionalLight.Direction), 0, 1);
  vec4 diffuseColor = vec4(directionalLight.Color, 1) * directionalLight.DiffuseIntensity * diffuseFactor;

  vec3 vertexToCamera = normalize(cameraPos - fragmentPos);
  vec3 lightReflect = normalize(reflect(directionalLight.Direction, normal));
  float specularFactor = pow(clamp(dot(vertexToCamera, lightReflect), 0, 1), materialSpecularFactor);
  vec4 specularColor = vec4(directionalLight.Color, 1) * materialSpecularIntensity * specularFactor;

  color = texture(textureSampler, fragmentUV) * (ambientColor + diffuseColor + specularColor);
}