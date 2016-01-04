#version 330 core

layout(location = 0) in vec2 vertexPos;
layout(location = 1) in vec2 vertexUV;

out vec2 fragmentUV;

void main() {
	fragmentUV = vertexUV;

    gl_Position.x = vertexPos.x*2 - 1;
    gl_Position.y = vertexPos.y*2 - 1;
    gl_Position.z = -1.0;
    gl_Position.w =  1.0;
}