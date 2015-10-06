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

struct SpotLight {
  vec3 direction;
  vec3 position;
  float cutoff;

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
uniform SpotLight spotLight;

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

  float pointLightDistance = length(pointLight.position - fragmentPos); // TODO: swap and create temp variable
  float pointLightDistance2 = 1.0 + pow(pointLightDistance, 2); // 1.0 constant prevents division by zero
  vec3 pointLightDirection = normalize(fragmentPos - pointLight.position);

  vec4 pointAmbientColor = vec4(pointLight.color, 1) * pointLight.ambientIntensity / pointLightDistance2;

  float pointDiffuseFactor = max(0.0, dot(normal, -pointLightDirection));
  vec4 pointDiffuseColor = vec4(pointLight.color, 1) * pointLight.diffuseIntensity * pointDiffuseFactor / pointLightDistance2;

  vec3 pointLightReflect = normalize(reflect(pointLightDirection, normal));
  float pointSpecularFactor = pow(max(0.0, dot(fragmentToCamera, pointLightReflect)), materialSpecularFactor);
  vec4 pointSpecularColor = pointLight.specularIntensity * vec4(pointLight.color, 1) * materialSpecularIntensity * pointSpecularFactor / pointLightDistance2;

  vec4 pointColor = pointAmbientColor + pointDiffuseColor + pointSpecularColor;

  // spot light TODO: make procedure

  vec3 spotLightToFragment = normalize(fragmentPos - spotLight.position);
  float spotAngleCos = dot(spotLightToFragment, spotLight.direction);
  float spotFactor = float(spotAngleCos > spotLight.cutoff) *(1.0 - 1.0*(1.0 - spotAngleCos) / (1.0 - spotLight.cutoff));

  float spotLightDistance = length(spotLight.position - fragmentPos);
  float spotLightDistance2 = 1.0 + pow(spotLightDistance, 2); // 1.0 constant prevents division by zero

  vec3 spotLightDirection = normalize(fragmentPos - spotLight.position);

  vec4 spotAmbientColor = vec4(spotLight.color, 1) * spotLight.ambientIntensity / spotLightDistance2;

  float spotDiffuseFactor = max(0.0, dot(normal, -spotLightDirection));
  vec4 spotDiffuseColor = vec4(spotLight.color, 1) * spotLight.diffuseIntensity * spotDiffuseFactor / spotLightDistance2;

  vec3 spotLightReflect = normalize(reflect(spotLightDirection, normal));
  float spotSpecularFactor = pow(max(0.0, dot(fragmentToCamera, spotLightReflect)), materialSpecularFactor);
  vec4 spotSpecularColor = spotLight.specularIntensity * vec4(spotLight.color, 1) * materialSpecularIntensity * spotSpecularFactor / spotLightDistance2;

  vec4 spotColor = spotFactor*(spotAmbientColor + spotDiffuseColor + spotSpecularColor);

  vec4 temp = texture(textureSampler, fragmentUV) * (directColor + pointColor + spotColor);
  color = pow(temp, gamma);
}