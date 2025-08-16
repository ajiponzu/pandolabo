#version 460 core

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushTimer {
  float time;
} push_timer;

void main() {
    outColor = vec4(fragColor + sin(push_timer.time) * cos(push_timer.time), 1.0);
}
