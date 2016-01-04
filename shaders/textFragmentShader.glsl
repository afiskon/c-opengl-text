#version 330 core

in vec2 fragmentUV;

out vec3 color;

uniform sampler2D textureSampler;

void main() {
    color = texture(textureSampler, fragmentUV).rgb;
}