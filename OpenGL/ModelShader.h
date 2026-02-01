#ifndef MODEL_SHADER_H
#define MODEL_SHADER_H


const char* modelVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // **CRITICAL: Proper normal transformation**
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    TexCoord = aTexCoord;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* modelFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

struct Light {
    vec3 position;
    vec3 color;
    float ambient;
    float diffuse;
    float specular;
};

uniform Light lights[4];
uniform int numLights;
uniform vec3 viewPos;
uniform vec3 objectColor;
uniform float shininess;
uniform int hasTexture;
uniform sampler2D texture_diffuse1;

void main() {
    // **CRITICAL: Normalize the interpolated normal**
    vec3 norm = normalize(Normal);
    vec3 result = vec3(0.0);
    
    // Enhanced lighting calculation
    for(int i = 0; i < numLights; i++) {
        vec3 lightDir = normalize(lights[i].position - FragPos);
        vec3 viewDir = normalize(viewPos - FragPos);
        
        // Ambient
        vec3 ambient = lights[i].ambient * lights[i].color * 0.3;
        
        // Diffuse with smoother falloff
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = lights[i].diffuse * diff * lights[i].color;
        
        // Specular with Blinn-Phong for smoother highlights
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
        vec3 specular = lights[i].specular * spec * lights[i].color * 0.5;
        
        result += (ambient + diffuse + specular);
    }
    
    // **IMPORTANT: Clamp the result to prevent over-brightness**
    result = clamp(result, 0.0, 1.0);
    
    vec3 finalColor = result * objectColor;
    FragColor = vec4(finalColor, 1.0);
}
)";


#endif