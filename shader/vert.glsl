#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

out vec4 fragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    fragColor = inColor;
    gl_Position = projection * view * model * vec4(inPosition, 1.0);
}
