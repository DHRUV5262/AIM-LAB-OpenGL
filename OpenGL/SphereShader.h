#ifndef SPHERE_SHADER_H
#define SPHERE_SHADER_H

// Vertex Shader for Sphere with multiple lights
const char* sphereVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec3 OurColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    // Calculate normal in world coordinates
    Normal = mat3(transpose(inverse(model))) * aNormal;
    OurColor = aColor;
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment Shader for Sphere with multiple lights
const char* sphereFragmentShaderSource = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec3 OurColor;

out vec4 FragColor;

#define MAX_LIGHTS 8

struct Light {
    vec3 position;
    vec3 color;
    
    // Light properties
    float ambient;
    float diffuse;
    float specular;
    
    // Attenuation
    float constant;
    float linear;
    float quadratic;
};

uniform Light lights[MAX_LIGHTS];
uniform int numLights;
uniform vec3 viewPos;   // Camera position for specular reflection
uniform float shininess;

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Initialize with global ambient light
    vec3 result = vec3(0.1) * OurColor;
    
    // Calculate contribution from each light
    for(int i = 0; i < numLights && i < MAX_LIGHTS; i++) {
        // Calculate direction and distance to light
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float distance = length(lights[i].position - FragPos);
        
        // Calculate attenuation
        float attenuation = 1.0 / (lights[i].constant + 
                                 lights[i].linear * distance + 
                                 lights[i].quadratic * distance * distance);
        
        // Ambient component
        vec3 ambient = lights[i].ambient * lights[i].color;
        
        // Diffuse component  
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = lights[i].diffuse * diff * lights[i].color;
        
        // Specular component
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = lights[i].specular * spec * lights[i].color;
        
        // Apply attenuation and add this light's contribution
        result += (ambient + diffuse + specular) * attenuation * OurColor;
    }
    
    // Apply tone mapping to prevent over-exposure when using multiple lights
    result = result / (result + vec3(1.0));
    
    FragColor = vec4(result, 1.0);
}
)";

// Enhanced Mirror Sphere Vertex Shader
const char* mirrorVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;
out vec3 ReflectDir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    Position = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pre-calculate reflection direction
    vec3 viewDir = normalize(Position - cameraPos);
    ReflectDir = reflect(viewDir, normalize(Normal));
    
    gl_Position = projection * view * worldPos;
}
)";

// Enhanced Mirror Sphere Fragment Shader
const char* mirrorFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec3 ReflectDir;

uniform vec3 cameraPos;
uniform samplerCube skybox;

void main() {
    // Sample the skybox with the reflection direction
    vec3 reflectedColor = texture(skybox, ReflectDir).rgb;
    
    // Add some Fresnel effect for more realistic reflection
    vec3 viewDir = normalize(Position - cameraPos);
    float fresnel = pow(1.0 - max(dot(-viewDir, normalize(Normal)), 0.0), 2.0);
    fresnel = mix(0.8, 1.0, fresnel); // Keep reflection strong but add some variation
    
    FragColor = vec4(reflectedColor * fresnel, 1.0);
}
)";

// Enhanced Transparent Sphere Vertex Shader
const char* transparentVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;
out vec3 ReflectDir;
out vec3 RefractDir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
uniform float refractionRatio;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    Position = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pre-calculate directions
    vec3 viewDir = normalize(Position - cameraPos);
    vec3 normalizedNormal = normalize(Normal);
    
    ReflectDir = reflect(viewDir, normalizedNormal);
    RefractDir = refract(viewDir, normalizedNormal, refractionRatio);
    
    gl_Position = projection * view * worldPos;
}
)";

// Enhanced Transparent Sphere Fragment Shader
const char* transparentFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec3 ReflectDir;
in vec3 RefractDir;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform float transparency; // Control transparency level (0.0 = opaque, 1.0 = fully transparent)

void main() {
    vec3 normalizedNormal = normalize(Normal);
    vec3 viewDir = normalize(Position - cameraPos);
    
    // Sample reflection
    vec3 reflectedColor = texture(skybox, ReflectDir).rgb;
    
    // Sample refraction - handle total internal reflection
    vec3 refractedColor;
    if (length(RefractDir) > 0.0) {
        refractedColor = texture(skybox, RefractDir).rgb;
    } else {
        // Total internal reflection - use reflection instead
        refractedColor = reflectedColor;
    }
    
    // Calculate Fresnel effect (more physically accurate)
    float cosTheta = max(dot(-viewDir, normalizedNormal), 0.0);
    float fresnel = pow(1.0 - cosTheta, 3.0);
    
    // Mix reflection and refraction based on Fresnel
    vec3 finalColor = mix(refractedColor, reflectedColor, fresnel * 0.4);
    
    // Add some base transparency and make it more glass-like
    float alpha = mix(0.3, 0.8, transparency); // Minimum transparency of 0.3
    
    // Add slight tint for glass effect
    finalColor = mix(finalColor, finalColor * vec3(0.9, 1.0, 0.95), 0.1);
    
    FragColor = vec4(finalColor, alpha);
}
)";

// Function to compile special shaders with improved error handling
int compileSpecialShader(const char* vertexSource, const char* fragmentSource, const char* shaderName) {
    int success;
    char infoLog[512];

    // Vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << shaderName << " vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    // Fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << shaderName << " fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    // Link program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << shaderName << " shader program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

#endif // SPHERE_SHADER_H