#version 330 core

in vec2 fragmentUV;
in vec3 fragmentNormal;
in vec3 fragmentPos;

struct DirectionalLight {
  vec3 direction;
  vec3 color;
  float ambientIntensity;
  float diffuseIntensity;
};

struct PointLight {
  vec3 position;
  vec3 color;
  float ambientIntensity;
  float diffuseIntensity;
};

uniform sampler2D textureSampler;
uniform vec3 cameraPos;
uniform float materialSpecularFactor;
uniform float materialSpecularIntensity;
uniform DirectionalLight directionalLight;
uniform PointLight pointLight;

out vec4 color;

void main() {
  vec3 normal = normalize(fragmentNormal); // normal should be corrected after interpolation
  vec4 gamma = vec4(vec3(1.0/2.2), 1);

  // direct light TODO: make procedure

  vec4 directAmbientColor = vec4(directionalLight.color, 1) * directionalLight.ambientIntensity;

  float directDiffuseFactor = max(0.0, dot(normal, -directionalLight.direction));
  vec4 directDiffuseColor = vec4(directionalLight.color, 1) * directionalLight.diffuseIntensity * directDiffuseFactor;

  vec3 vertexToCamera = normalize(cameraPos - fragmentPos);
  vec3 lightReflect = normalize(reflect(directionalLight.direction, normal));
  float specularFactor = pow(max(0.0, dot(vertexToCamera, lightReflect)), materialSpecularFactor);
  vec4 directSpecularColor = vec4(directionalLight.color, 1) * materialSpecularIntensity * specularFactor;

  vec4 directColor = directAmbientColor + directDiffuseColor + directSpecularColor;

  // point light TODO: make procedure

  float distance = length(pointLight.position - fragmentPos);
  float distance2 = distance * distance;

  vec4 pointAmbientColor = vec4(pointLight.color, 1) * pointLight.ambientIntensity / distance2;

  vec3 pointLightDirection = normalize(fragmentPos - pointLight.position);
  float pointDiffuseFactor = max(0.0, dot(normal, -pointLightDirection));
  vec4 pointDiffuseColor = vec4(pointLight.color, 1) * pointLight.diffuseIntensity * pointDiffuseFactor / distance2;
//
//  vec3 vertexToCamera = normalize(cameraPos - fragmentPos);
//  vec3 lightReflect = normalize(reflect(directionalLight.direction, normal));
//  float specularFactor = pow(max(0.0, dot(vertexToCamera, lightReflect)), materialSpecularFactor);
//  vec4 directSpecularColor = vec4(directionalLight.color, 1) * materialSpecularIntensity * specularFactor;

  vec4 pointColor = pointAmbientColor + pointDiffuseColor; //+ directSpecularColor;


  vec4 temp = texture(textureSampler, fragmentUV) * (directColor + pointColor);
  color = pow(temp, gamma);
}