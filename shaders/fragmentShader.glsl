#version 330 core

in vec2 fragmentUV;
in vec3 fragmentNormal;
in vec3 fragmentPos;

struct DirectionalLight {
  vec3 direction;

  vec3 color;
  float ambientIntensity;
  float diffuseIntensity;
  float specularIntensity; // for debug purposes, should be set to 1.0
};

struct PointLight {
  vec3 position;

  vec3 color;
  float ambientIntensity;
  float diffuseIntensity;
  float specularIntensity; // for debug purposes, should be set to 1.0
};

uniform sampler2D textureSampler;
uniform vec3 cameraPos;
uniform float materialSpecularFactor; // should be >= 1.0
uniform float materialSpecularIntensity;
uniform DirectionalLight directionalLight;
uniform PointLight pointLight;

out vec4 color;

void main() {
  vec3 normal = normalize(fragmentNormal); // normal should be corrected after interpolation
  vec4 gamma = vec4(vec3(1.0/2.2), 1);
  vec3 fragmentToCamera = normalize(cameraPos - fragmentPos);

  // direct light TODO: make procedure

  vec4 directAmbientColor = vec4(directionalLight.color, 1) * directionalLight.ambientIntensity;

  float directDiffuseFactor = max(0.0, dot(normal, -directionalLight.direction));
  vec4 directDiffuseColor = vec4(directionalLight.color, 1) * directionalLight.diffuseIntensity * directDiffuseFactor;


  vec3 directLightReflect = normalize(reflect(directionalLight.direction, normal));
  float directSpecularFactor = pow(max(0.0, dot(fragmentToCamera, directLightReflect)), materialSpecularFactor);
  vec4 directSpecularColor = directionalLight.specularIntensity * vec4(directionalLight.color, 1) * materialSpecularIntensity * directSpecularFactor;

  vec4 directColor = directAmbientColor + directDiffuseColor + directSpecularColor;

  // point light TODO: make procedure

  float distance = length(pointLight.position - fragmentPos);
  float distance2 = 1.0 + pow(distance, 2); // 1.0 constant prevents from division by zero
  vec3 pointLightDirection = normalize(fragmentPos - pointLight.position);

  vec4 pointAmbientColor = vec4(pointLight.color, 1) * pointLight.ambientIntensity / distance2;

  float pointDiffuseFactor = max(0.0, dot(normal, -pointLightDirection));
  vec4 pointDiffuseColor = vec4(pointLight.color, 1) * pointLight.diffuseIntensity * pointDiffuseFactor / distance2;

  vec3 pointLightReflect = normalize(reflect(pointLightDirection, normal));
  float pointSpecularFactor = pow(max(0.0, dot(fragmentToCamera, pointLightReflect)), materialSpecularFactor);
  vec4 pointSpecularColor = pointLight.specularIntensity * vec4(pointLight.color, 1) * materialSpecularIntensity * pointSpecularFactor / distance2;

  vec4 pointColor = pointAmbientColor + pointDiffuseColor + pointSpecularColor;

  vec4 temp = texture(textureSampler, fragmentUV) * (directColor + pointColor);
  color = pow(temp, gamma);
}