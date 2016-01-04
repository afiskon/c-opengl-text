#version 330 core

in vec2 fragmentUV;

out vec4 color;

uniform sampler2D textureSampler;

void main() {
    color = texture(textureSampler, fragmentUV);
    color = vec4(1, 1, 1, color.a);
}