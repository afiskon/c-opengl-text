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
  vec4 gamma = vec4(vec3(1.0/2.2), 1);

  vec4 directAmbientColor = vec4(directionalLight.Color, 1) * directionalLight.AmbientIntensity;

  float diffuseFactor = max(0.0, dot(normal, -directionalLight.Direction));
  vec4 directDiffuseColor = vec4(directionalLight.Color, 1) * directionalLight.DiffuseIntensity * diffuseFactor;

  vec3 vertexToCamera = normalize(cameraPos - fragmentPos);
  vec3 lightReflect = normalize(reflect(directionalLight.Direction, normal));
  float specularFactor = pow(max(0.0, dot(vertexToCamera, lightReflect)), materialSpecularFactor);
  vec4 directSpecularColor = vec4(directionalLight.Color, 1) * materialSpecularIntensity * specularFactor;

  vec4 directColor = directAmbientColor + directDiffuseColor + directSpecularColor;

  vec4 temp = texture(textureSampler, fragmentUV) * directColor;
  color = pow(temp, gamma);
}