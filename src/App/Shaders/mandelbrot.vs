#version 330 core

layout(location=0) in vec2 pos;

out vec2 fragCoord;

void main() {
    fragCoord = pos;
    gl_Position = vec4(pos, 0.0, 1.0);
}
