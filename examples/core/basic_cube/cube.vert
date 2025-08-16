#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform CubePosition {
  mat4 model;
  mat4 view;
  mat4 proj;
} mvp;

void main() {
    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}
