#pragma once
#ifndef SIMPLE_LIGHT_SHADER_H
#define SIMPLE_LIGHT_SHADER_H

// Simple vertex shader for light visualization
const char* lightVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal; // Not used but needed for compatibility
layout (location = 2) in vec3 aColor;

out vec3 OurColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    OurColor = aColor;
}
)";

// Simple fragment shader for light visualization (emissive)
const char* lightFragmentShaderSource = R"(
#version 330 core
in vec3 OurColor;
out vec4 FragColor;

void main() {
    // Make light sources appear emissive by adding extra brightness
    vec3 brightColor = OurColor * 3;
    FragColor = vec4(brightColor, 5.0;
}
)";

#endif // SIMPLE_LIGHT_SHADER_H