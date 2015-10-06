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

vec4 calcDirectionalLight(vec3 normal, vec3 fragmentToCamera) {
  vec4 ambientColor = vec4(directionalLight.color, 1) * directionalLight.ambientIntensity;

  float diffuseFactor = max(0.0, dot(normal, -directionalLight.direction));
  vec4 diffuseColor = vec4(directionalLight.color, 1) * directionalLight.diffuseIntensity * diffuseFactor;

  vec3 lightReflect = normalize(reflect(directionalLight.direction, normal));
  float specularFactor = pow(max(0.0, dot(fragmentToCamera, lightReflect)), materialSpecularFactor);
  vec4 specularColor = directionalLight.specularIntensity * vec4(directionalLight.color, 1) * materialSpecularIntensity * specularFactor;

  return ambientColor + diffuseColor + specularColor;
}

vec4 calcPointLight(vec3 normal, vec3 fragmentToCamera) {
  float distance = length(fragmentPos - pointLight.position);
  float distance2 = 1.0 + pow(distance, 2); // 1.0 constant prevents division by zero
  vec3 lightDirection = normalize(fragmentPos - pointLight.position);

  vec4 ambientColor = vec4(pointLight.color, 1) * pointLight.ambientIntensity / distance2;

  float diffuseFactor = max(0.0, dot(normal, -lightDirection));
  vec4 diffuseColor = vec4(pointLight.color, 1) * pointLight.diffuseIntensity * diffuseFactor / distance2;

  vec3 lightReflect = normalize(reflect(lightDirection, normal));
  float specularFactor = pow(max(0.0, dot(fragmentToCamera, lightReflect)), materialSpecularFactor);
  vec4 specularColor = pointLight.specularIntensity * vec4(pointLight.color, 1) * materialSpecularIntensity * specularFactor / distance2;

  return ambientColor + diffuseColor + specularColor;
}


vec4 calcSpotLight(vec3 normal, vec3 fragmentToCamera) {
  vec3 spotLightDirection = normalize(fragmentPos - spotLight.position);
  float spotAngleCos = dot(spotLightDirection, spotLight.direction);
  float spotFactor = float(spotAngleCos > spotLight.cutoff) *(1.0 - 1.0*(1.0 - spotAngleCos) / (1.0 - spotLight.cutoff));

  float spotLightDistance = length(fragmentPos - spotLight.position);
  float spotLightDistance2 = 1.0 + pow(spotLightDistance, 2); // 1.0 constant prevents division by zero

  vec4 spotAmbientColor = vec4(spotLight.color, 1) * spotLight.ambientIntensity / spotLightDistance2;

  float spotDiffuseFactor = max(0.0, dot(normal, -spotLightDirection));
  vec4 spotDiffuseColor = vec4(spotLight.color, 1) * spotLight.diffuseIntensity * spotDiffuseFactor / spotLightDistance2;

  vec3 spotLightReflect = normalize(reflect(spotLightDirection, normal));
  float spotSpecularFactor = pow(max(0.0, dot(fragmentToCamera, spotLightReflect)), materialSpecularFactor);
  vec4 spotSpecularColor = spotLight.specularIntensity * vec4(spotLight.color, 1) * materialSpecularIntensity * spotSpecularFactor / spotLightDistance2;

  return spotFactor*(spotAmbientColor + spotDiffuseColor + spotSpecularColor);
}

void main() {
  vec3 normal = normalize(fragmentNormal); // normal should be corrected after interpolation
  vec3 fragmentToCamera = normalize(cameraPos - fragmentPos);

  vec4 directColor = calcDirectionalLight(normal, fragmentToCamera);
  vec4 pointColor = calcPointLight(normal, fragmentToCamera);
  vec4 spotColor = calcSpotLight(normal, fragmentToCamera);
  vec4 linearColor = texture(textureSampler, fragmentUV) * (directColor + pointColor + spotColor);

  vec4 gamma = vec4(vec3(1.0/2.2), 1);
  color = pow(linearColor, gamma); // gamma-corrected color
}