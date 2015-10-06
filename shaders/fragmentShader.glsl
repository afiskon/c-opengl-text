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

vec4 calcDirectionalLight(vec3 normal, vec3 fragmentToCamera, DirectionalLight light) {
  vec4 ambientColor = vec4(light.color, 1) * light.ambientIntensity;

  float diffuseFactor = max(0.0, dot(normal, -light.direction));
  vec4 diffuseColor = vec4(light.color, 1) * light.diffuseIntensity * diffuseFactor;

  vec3 lightReflect = normalize(reflect(light.direction, normal));
  float specularFactor = pow(max(0.0, dot(fragmentToCamera, lightReflect)), materialSpecularFactor);
  vec4 specularColor = light.specularIntensity * vec4(light.color, 1) * materialSpecularIntensity * specularFactor;

  return ambientColor + diffuseColor + specularColor;
}

vec4 calcPointLight(vec3 normal, vec3 fragmentToCamera, PointLight light) {
  float distance = length(fragmentPos - light.position);
  float distance2 = 1.0 + pow(distance, 2); // 1.0 constant prevents division by zero
  vec3 lightDirection = normalize(fragmentPos - light.position);

  vec4 ambientColor = vec4(light.color, 1) * light.ambientIntensity / distance2;

  float diffuseFactor = max(0.0, dot(normal, -lightDirection));
  vec4 diffuseColor = vec4(light.color, 1) * light.diffuseIntensity * diffuseFactor / distance2;

  vec3 lightReflect = normalize(reflect(lightDirection, normal));
  float specularFactor = pow(max(0.0, dot(fragmentToCamera, lightReflect)), materialSpecularFactor);
  vec4 specularColor = light.specularIntensity * vec4(light.color, 1) * materialSpecularIntensity * specularFactor / distance2;

  return ambientColor + diffuseColor + specularColor;
}

vec4 calcSpotLight(vec3 normal, vec3 fragmentToCamera, SpotLight light) {
  vec3 spotLightDirection = normalize(fragmentPos - light.position);
  float spotAngleCos = dot(spotLightDirection, light.direction);
  float spotFactor = float(spotAngleCos > light.cutoff) * (1.0 - 1.0*(1.0 - spotAngleCos) / (1.0 - light.cutoff));

  PointLight tempPointLight = PointLight(light.position, light.color, light.ambientIntensity, light.diffuseIntensity, light.ambientIntensity);
  return spotFactor*calcPointLight(normal, fragmentToCamera, tempPointLight);
}

void main() {
  vec3 normal = normalize(fragmentNormal); // normal should be corrected after interpolation
  vec3 fragmentToCamera = normalize(cameraPos - fragmentPos);

  vec4 directColor = calcDirectionalLight(normal, fragmentToCamera, directionalLight);
  vec4 pointColor = calcPointLight(normal, fragmentToCamera, pointLight);
  vec4 spotColor = calcSpotLight(normal, fragmentToCamera, spotLight);
  vec4 linearColor = texture(textureSampler, fragmentUV) * (directColor + pointColor + spotColor);

  vec4 gamma = vec4(vec3(1.0/2.2), 1);
  color = pow(linearColor, gamma); // gamma-corrected color
}