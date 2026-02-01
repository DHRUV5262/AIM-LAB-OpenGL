#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <glm/gtc/type_ptr.hpp>
#include "Sphere.h"
#include "SphereShader.h"
#include "Light.h"
#include "SimpleLightShader.h"
#include "Model.h"
#include "ModelShader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

// Global variables for game state
int score = 0;
std::mt19937 rng(std::random_device{}());
std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);
std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);
std::uniform_real_distribution<float> sizeDist(0.3f, 0.8f);

// Crosshair shader sources
const char* crosshairVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* crosshairFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 0.8);
}
)";

// Vertex Shader
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 OurColor;
uniform float time;

void main() {
    mat2 rotation = mat2(cos(time), -sin(time),
                        sin(time),  cos(time));
    vec2 rotated = rotation * aPos.xy;
    gl_Position = vec4(rotated, aPos.z, 1.0);
    OurColor = aColor;
}
)";

// Fragment Shader
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 OurColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(OurColor, 1.0);
}
)";

// Skybox Vertex Shader - Updated with correct coordinate handling
const char* skyboxVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
out vec3 TexCoords;
uniform mat4 projection;
uniform mat4 view;
void main() {
    TexCoords = aPos;
    gl_Position = projection * view * vec4(aPos, 1.0);
    // Ensure depth is 1.0 (maximum depth)
    gl_Position = gl_Position.xyww;
}
)";

// Fragment Shader - No changes needed here
const char* skyboxFragmentShaderSource = R"(
#version 330 core
in vec3 TexCoords;
out vec4 FragColor;
uniform samplerCube skybox;
void main() {
    FragColor = texture(skybox, TexCoords);
}
)";

// Skybox vertices - Using larger size to ensure visibility
float skyboxVertices[] = {
    // positions          
    -10.0f,  10.0f, -10.0f,
    -10.0f, -10.0f, -10.0f,
     10.0f, -10.0f, -10.0f,
     10.0f,  10.0f, -10.0f,
    -10.0f, -10.0f,  10.0f,
    -10.0f,  10.0f,  10.0f,
     10.0f, -10.0f,  10.0f,
     10.0f,  10.0f,  10.0f
};

// Skybox indices
unsigned int skyboxIndices[] = {
    // Back face
    0, 1, 3, 3, 1, 2,
    // Left face
    5, 1, 0, 5, 4, 1,
    // Front face
    7, 6, 4, 7, 4, 5,
    // Right face
    3, 2, 7, 7, 2, 6,
    // Top face
    5, 0, 7, 7, 0, 3,
    // Bottom face
    1, 4, 2, 2, 4, 6
};

// Crosshair vertices (simple cross)
float crosshairVertices[] = {
    // Horizontal line
    -0.02f,  0.0f,
     0.02f,  0.0f,
     // Vertical line
      0.0f, -0.03f,
      0.0f,  0.03f
};

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = 0.0f;
float pitch = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
bool firstMouse = true;
float sensitivity = 0.1f;


// Generate random sphere position
glm::vec3 generateRandomPosition() {
    return glm::vec3(posDist(rng), posDist(rng), posDist(rng));
}

// Generate random sphere color
glm::vec3 generateRandomColor() {
    return glm::vec3(colorDist(rng), colorDist(rng), colorDist(rng));
}

// Generate random sphere size
float generateRandomSize() {
    std::uniform_real_distribution<float> sizeDist(2.0f, 4.0f);
    return sizeDist(rng);
}

void processInput(GLFWwindow* window) {
    static float deltaTime = 0.0f;
    static float lastFrame = 0.0f;
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // Add debug key
    static bool vKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && !vKeyPressed) {
        vKeyPressed = true;
        std::vector<Sphere>* spheres = static_cast<std::vector<Sphere>*>(glfwGetWindowUserPointer(window));

    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        vKeyPressed = false;
    }
}



void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = (xpos - lastX) * sensitivity;
    float yoffset = (lastY - ypos) * sensitivity;
    lastX = xpos;
    lastY = ypos;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Mouse button callback for shooting
// Replace your existing raySphereIntersectionSimple function with this improved version
// Fixed ray-sphere intersection function


// CORRECTED ray-sphere intersection function
bool raySphereIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
    const glm::vec3& sphereCenter, float sphereRadius, float* t_out = nullptr) {

    // Vector from ray origin to sphere center
    glm::vec3 oc = rayOrigin - sphereCenter;

    // Quadratic equation coefficients: at² + bt + c = 0
    // where t is the parameter along the ray
    float a = glm::dot(rayDir, rayDir);  // Should be 1.0 for normalized ray
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;

    // Calculate discriminant
    float discriminant = b * b - 4 * a * c;

    // Debug output with CORRECT format
    std::cout << "    === Ray-Sphere Intersection Debug ===" << std::endl;
    std::cout << "    Ray Origin: (" << rayOrigin.x << ", " << rayOrigin.y << ", " << rayOrigin.z << ")" << std::endl;
    std::cout << "    Ray Direction: (" << rayDir.x << ", " << rayDir.y << ", " << rayDir.z << ")" << std::endl;
    std::cout << "    Sphere Center: (" << sphereCenter.x << ", " << sphereCenter.y << ", " << sphereCenter.z << ")" << std::endl;
    std::cout << "    Sphere Radius: " << sphereRadius << std::endl;
    std::cout << "    Distance to sphere center: " << glm::length(oc) << std::endl;
    std::cout << "    a: " << a << ", b: " << b << ", c: " << c << std::endl;
    std::cout << "    Discriminant: " << discriminant << std::endl;

    if (discriminant < 0) {
        std::cout << "    No intersection (discriminant < 0)" << std::endl;
        return false;
    }

    // Calculate both intersection points
    float sqrtDiscriminant = sqrt(discriminant);
    float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
    float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

    std::cout << "    t1: " << t1 << ", t2: " << t2 << std::endl;

    // We want the closest positive intersection (in front of camera)
    float t = (t1 > 0) ? t1 : t2;

    if (t > 0) {
        glm::vec3 intersectionPoint = rayOrigin + t * rayDir;
        std::cout << "    Intersection at t=" << t << ", point: ("
            << intersectionPoint.x << ", " << intersectionPoint.y << ", " << intersectionPoint.z << ")" << std::endl;
        std::cout << "    HIT DETECTED!" << std::endl;

        if (t_out) *t_out = t;
        return true;
    }

    std::cout << "    No positive intersection" << std::endl;
    return false;
}
void Hitting(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
    const glm::vec3& sphereCenter, float sphereRadius, float tolerance = 0.5f) {

    // Ensure ray direction is normalized
    glm::vec3 normalizedRayDir = rayDir;

    // Vector from ray origin to sphere center
    glm::vec3 toCenter = sphereCenter - rayOrigin;

    // Project toCenter onto the ray direction to find closest point
    float projection = glm::dot(toCenter, normalizedRayDir);

    // Calculate the closest point on the ray to the sphere center
    glm::vec3 closestPoint = rayOrigin + projection * normalizedRayDir;

    // Calculate distance from closest point to sphere center
    float distToCenter = glm::length(closestPoint - sphereCenter);

    // Check if we're within tolerance
    bool hit = distToCenter <= (sphereRadius + tolerance);

    // Debug output
    std::cout << "    === Hitscan with Tolerance Debug ===" << std::endl;
    std::cout << "    Projection: " << projection << std::endl;
    std::cout << "    Closest point: (" << closestPoint.x << ", " << closestPoint.y << ", " << closestPoint.z << ")" << std::endl;
    std::cout << "    Distance to center: " << distToCenter << std::endl;
    std::cout << "    Radius + tolerance: " << (sphereRadius + tolerance) << std::endl;
    std::cout << "    Hit: " << (hit ? "YES" : "NO") << std::endl;

  
}

// Updated mouse button callback with corrected intersection and single sphere
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        std::cout << "\n*** LEFT CLICK DETECTED ***" << std::endl;

        std::vector<Sphere>* spheres = static_cast<std::vector<Sphere>*>(glfwGetWindowUserPointer(window));

        if (spheres && !spheres->empty()) {
            std::cout << "Number of spheres: " << spheres->size() << std::endl;

            // Use direct ray from camera (simpler and more reliable)
            glm::vec3 rayOrigin = cameraPos;
            glm::vec3 rayDir = glm::normalize(cameraFront);

            std::cout << "Camera Position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
            std::cout << "Camera Front: (" << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << ")" << std::endl;

            // Check intersection with the single sphere
            bool hitAny = false;
            float closestDistance = FLT_MAX;
            int closestSphereIndex = -1;
            float hitDistance = 0;

            for (size_t i = 0; i < spheres->size(); ++i) {
                Sphere& sphere = (*spheres)[i];
                std::cout << "Checking sphere " << i << ":" << std::endl;

                float t;
                bool hit = raySphereIntersection(rayOrigin, rayDir, sphere.getPosition(), sphere.getRadius() + 1.0f, &t);

                Hitting(rayOrigin, rayDir, sphere.getPosition(), sphere.getRadius() + 1.0f);
                std::cout << "    Method 1 hit: " << (hit ? 1 : 0) << ", Method 2 hit: " << (hit ? 1 : 0) << std::endl;

                if (hit && t < closestDistance) {
                    closestDistance = t;
                    closestSphereIndex = i;
                    hitDistance = t;
                    hitAny = true;
                }
            }

            if (hitAny && closestSphereIndex >= 0) {
                Sphere& hitSphere = (*spheres)[closestSphereIndex];
                score += 10;
                std::cout << "*** HIT SPHERE " << closestSphereIndex << "! *** Score: " << score << std::endl;

                // Respawn sphere at random location with random properties
                hitSphere.setPosition(generateRandomPosition());
                hitSphere.setColor(generateRandomColor());
                hitSphere.setRadius(generateRandomSize());
            }
            else {
                std::cout << "No spheres hit." << std::endl;
            }
        }
        std::cout << "*** END CLICK HANDLING ***\n" << std::endl;
    }
}

void renderGunModel(unsigned int modelShaderProgram, Model& gunModel,
    const glm::mat4& view, const glm::mat4& projection,
    const std::vector<Light>& lights, const glm::vec3& cameraPos,
    const glm::vec3& cameraFront, const glm::vec3& cameraUp) {

    glUseProgram(modelShaderProgram);

    // Appropriate scale for weapon
    gunModel.setScale(glm::vec3(0.08f, 0.08f, 0.08f));

    // **CRITICAL: Set material properties for proper lighting**
    glUniform3fv(glGetUniformLocation(modelShaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

    // Gun material properties (metallic/matte finish)
    glm::vec3 gunColor = glm::vec3(0.15f, 0.15f, 0.15f); // Dark gunmetal
    glUniform3fv(glGetUniformLocation(modelShaderProgram, "objectColor"), 1, glm::value_ptr(gunColor));
    glUniform1f(glGetUniformLocation(modelShaderProgram, "shininess"), 64.0f); // Moderate shine

    // Enable texture if available
    glUniform1i(glGetUniformLocation(modelShaderProgram, "hasTexture"), 0);

    // **ESSENTIAL: Update lighting uniforms**
    glUniform1i(glGetUniformLocation(modelShaderProgram, "numLights"), std::min((int)lights.size(), 4));

    for (size_t i = 0; i < lights.size() && i < 4; i++) {
        std::string lightBase = "lights[" + std::to_string(i) + "]";
        glUniform3fv(glGetUniformLocation(modelShaderProgram, (lightBase + ".position").c_str()),
            1, glm::value_ptr(lights[i].getPosition()));
        glUniform3fv(glGetUniformLocation(modelShaderProgram, (lightBase + ".color").c_str()),
            1, glm::value_ptr(lights[i].getColor()));
        glUniform1f(glGetUniformLocation(modelShaderProgram, (lightBase + ".ambient").c_str()),
            lights[i].getAmbient());
        glUniform1f(glGetUniformLocation(modelShaderProgram, (lightBase + ".diffuse").c_str()),
            lights[i].getDiffuse());
        glUniform1f(glGetUniformLocation(modelShaderProgram, (lightBase + ".specular").c_str()),
            lights[i].getSpecular());
    }

    // Draw the gun with proper matrices
    gunModel.draw(modelShaderProgram, view, projection);
}

// Additional helper function to visualize sphere positions relative to camera
void debugSphereVisibility(const std::vector<Sphere>& spheres) {
    std::cout << "\n=== SPHERE VISIBILITY DEBUG ===" << std::endl;
    std::cout << "Camera at: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "Looking at: (" << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << ")" << std::endl;

    for (size_t i = 0; i < spheres.size(); ++i) {
        const Sphere& sphere = spheres[i];
        glm::vec3 toSphere = sphere.getPosition() - cameraPos;
        float distance = glm::length(toSphere);
        float dotProduct = glm::dot(glm::normalize(toSphere), cameraFront);

        std::cout << "Sphere " << i << ":" << std::endl;
        std::cout << "  Position: (" << sphere.getPosition().x << ", " << sphere.getPosition().y << ", " << sphere.getPosition().z << ")" << std::endl;
        std::cout << "  Distance: " << distance << std::endl;
        std::cout << "  Dot product (forward alignment): " << dotProduct << " (>0.95 = very close to crosshair)" << std::endl;
        std::cout << "  Radius: " << sphere.getRadius() << std::endl;

        if (dotProduct > 0.95f && distance < 10.0f) {
            std::cout << "  >>> This sphere should be VERY close to crosshair <<<" << std::endl;
        }
        else if (dotProduct > 0.8f && distance < 10.0f) {
            std::cout << "  >>> This sphere should be visible and potentially clickable <<<" << std::endl;
        }
    }
    std::cout << "=== END VISIBILITY DEBUG ===\n" << std::endl;
}

glm::vec3 calculateGunRotationEuler(const glm::vec3& cameraFront) {
    return glm::vec3(pitch, yaw, 0.0f);
}



void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Debug function to print out image loading errors
void debugImageLoading(const std::string& path) {
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cout << "Failed to load image: " << path << std::endl;
        std::cout << "Error: " << stbi_failure_reason() << std::endl;
    }
    else {
        std::cout << "Successfully loaded image: " << path << std::endl;
        std::cout << "Size: " << width << "x" << height << ", Channels: " << channels << std::endl;
        stbi_image_free(data);
    }
}

// Load cubemap with enhanced error reporting
unsigned int loadCubemap(std::vector<std::string> faces) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    stbi_set_flip_vertically_on_load(false);

    int width, height, nrChannels;
    bool loadedAny = false;

    for (unsigned int i = 0; i < faces.size(); i++) {
        std::cout << "Loading face: " << faces[i] << std::endl;
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            loadedAny = true;
            GLenum format = GL_RGB;
            if (nrChannels == 1)
                format = GL_RED;
            else if (nrChannels == 3)
                format = GL_RGB;
            else if (nrChannels == 4)
                format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
                width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            std::cout << "  Success - Format: " << (format == GL_RGB ? "RGB" :
                (format == GL_RGBA ? "RGBA" : "Other")) << std::endl;
        }
        else {
            std::cout << "  Failed to load texture: " << faces[i] << std::endl;
            std::cout << "  Reason: " << stbi_failure_reason() << std::endl;
        }
    }

    if (!loadedAny) {
        std::cout << "Failed to load ANY skybox textures!" << std::endl;
        unsigned char fallback[3] = { 255, 0, 255 };
        for (unsigned int i = 0; i < 6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1, 1, 0,
                GL_RGB, GL_UNSIGNED_BYTE, fallback);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Aim Lab - Score: 0", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Compile crosshair shaders
    unsigned int crosshairVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(crosshairVertexShader, 1, &crosshairVertexShaderSource, NULL);
    glCompileShader(crosshairVertexShader);

    unsigned int crosshairFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(crosshairFragmentShader, 1, &crosshairFragmentShaderSource, NULL);
    glCompileShader(crosshairFragmentShader);

    unsigned int crosshairShaderProgram = glCreateProgram();
    glAttachShader(crosshairShaderProgram, crosshairVertexShader);
    glAttachShader(crosshairShaderProgram, crosshairFragmentShader);
    glLinkProgram(crosshairShaderProgram);

    glDeleteShader(crosshairVertexShader);
    glDeleteShader(crosshairFragmentShader);

    // Setup crosshair VAO
    unsigned int crosshairVAO, crosshairVBO;
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Compile and check shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Vertex data for rectangle (keeping for reference)
    float vertices[] = {
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f, 1.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);

    // Setup skybox VAO
    unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glGenBuffers(1, &skyboxEBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), skyboxIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Initialize skybox shader
    unsigned int skyboxVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(skyboxVertShader, 1, &skyboxVertexShaderSource, NULL);
    glCompileShader(skyboxVertShader);
    glGetShaderiv(skyboxVertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(skyboxVertShader, 512, NULL, infoLog);
        std::cerr << "Skybox vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int skyboxFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(skyboxFragShader, 1, &skyboxFragmentShaderSource, NULL);
    glCompileShader(skyboxFragShader);
    glGetShaderiv(skyboxFragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(skyboxFragShader, 512, NULL, infoLog);
        std::cerr << "Skybox fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int skyboxShader = glCreateProgram();
    glAttachShader(skyboxShader, skyboxVertShader);
    glAttachShader(skyboxShader, skyboxFragShader);
    glLinkProgram(skyboxShader);
    glGetProgramiv(skyboxShader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(skyboxShader, 512, NULL, infoLog);
        std::cerr << "Skybox shader program linking failed:\n" << infoLog << std::endl;
    }
    glDeleteShader(skyboxVertShader);
    glDeleteShader(skyboxFragShader);

    // Compile sphere shaders
    unsigned int sphereVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(sphereVertShader, 1, &sphereVertexShaderSource, NULL);
    glCompileShader(sphereVertShader);
    glGetShaderiv(sphereVertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(sphereVertShader, 512, NULL, infoLog);
        std::cerr << "Sphere vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int sphereFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(sphereFragShader, 1, &sphereFragmentShaderSource, NULL);
    glCompileShader(sphereFragShader);
    glGetShaderiv(sphereFragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(sphereFragShader, 512, NULL, infoLog);
        std::cerr << "Sphere fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int sphereShaderProgram = glCreateProgram();
    glAttachShader(sphereShaderProgram, sphereVertShader);
    glAttachShader(sphereShaderProgram, sphereFragShader);
    glLinkProgram(sphereShaderProgram);
    glGetProgramiv(sphereShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(sphereShaderProgram, 512, NULL, infoLog);
        std::cerr << "Sphere shader program linking failed:\n" << infoLog << std::endl;
    }
    glDeleteShader(sphereVertShader);
    glDeleteShader(sphereFragShader);

    // Define skybox texture paths
    std::vector<std::string> faces = {
        "skybox/right.jpg",
        "skybox/left.jpg",
        "skybox/top.jpg",
        "skybox/bottom.jpg",
        "skybox/front.jpg",
        "skybox/back.jpg"
    };

    std::cout << "Attempting to load skybox images..." << std::endl;
    for (const auto& path : faces) {
        debugImageLoading(path);
    }

    unsigned int cubemapTexture = loadCubemap(faces);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // Compile model shaders
    unsigned int modelVertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(modelVertShader, 1, &modelVertexShaderSource, NULL);
    glCompileShader(modelVertShader);
    glGetShaderiv(modelVertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(modelVertShader, 512, NULL, infoLog);
        std::cerr << "Model vertex shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int modelFragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(modelFragShader, 1, &modelFragmentShaderSource, NULL);
    glCompileShader(modelFragShader);
    glGetShaderiv(modelFragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(modelFragShader, 512, NULL, infoLog);
        std::cerr << "Model fragment shader compilation failed:\n" << infoLog << std::endl;
    }

    unsigned int modelShaderProgram = glCreateProgram();
    glAttachShader(modelShaderProgram, modelVertShader);
    glAttachShader(modelShaderProgram, modelFragShader);
    glLinkProgram(modelShaderProgram);
    glGetProgramiv(modelShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(modelShaderProgram, 512, NULL, infoLog);
        std::cerr << "Model shader program linking failed:\n" << infoLog << std::endl;
    }
    glDeleteShader(modelVertShader);
    glDeleteShader(modelFragShader);

    // Load gun model (place your .obj file in the project directory)
    Model gunModel("Model/M9.obj");

    // Position the gun in bottom-left of screen (relative to camera)
    gunModel.setPosition(glm::vec3(-0.5f, -0.3f, -2.0f)); // Left, down, close to camera
    gunModel.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));     // No rotation initially
    gunModel.setScale(glm::vec3(0.1f, 0.1f, 0.1f));        // Scale down


    // Create sphere objects with random properties
    //std::vector<Sphere> spheres;
    //for (int i = 0; i < 1; ++i) {
    //    glm::vec3 pos = generateRandomPosition();
    //    float radius = generateRandomSize();
    //    glm::vec3 color = generateRandomColor();

    //    spheres.push_back(Sphere(pos, radius, 36, 18));
    //    spheres.back().setColor(color);
    //    spheres.back().setup();
    //}

    //// Set spheres as window user pointer for mouse callback
    //glfwSetWindowUserPointer(window, &spheres);

    // Create multiple lights
    std::vector<Light> lights;
    lights.push_back(Light(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.1f, 0.8f, 1.0f));
    lights.push_back(Light(glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.2f, 0.2f), 0.1f, 0.6f, 0.8f));
    lights.push_back(Light(glm::vec3(-3.0f, 0.0f, 0.0f), glm::vec3(0.2f, 0.2f, 1.0f), 0.1f, 0.6f, 0.8f));
    lights.push_back(Light(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.2f, 1.0f, 0.2f), 0.1f, 0.6f, 0.8f));

    // Variables for window title updates
    std::string baseTitle = "Aim Lab - Score: ";
    int lastScore = -1;


    std::vector<Sphere> spheres;

    // Place sphere at a predictable location for testing
    glm::vec3 testPosition(0.0f, 0.0f, -2.0f);  // Right in front of camera
    float testRadius = 0.5f;
    glm::vec3 testColor(1.0f, 0.0f, 0.0f);  // Red color

    spheres.push_back(Sphere(testPosition, testRadius, 36, 18));
    spheres.back().setColor(testColor);
    spheres.back().setup();

    std::cout << "Created test sphere at: (" << testPosition.x << ", " << testPosition.y << ", " << testPosition.z << ")" << std::endl;
    std::cout << "Sphere radius: " << testRadius << std::endl;
    std::cout << "Initial camera position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;

    // Set spheres as window user pointer for mouse callback
    glfwSetWindowUserPointer(window, &spheres);


    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Update window title if score changed
        if (score != lastScore) {
            std::string newTitle = baseTitle + std::to_string(score);
            glfwSetWindowTitle(window, newTitle.c_str());
            lastScore = score;
        }

        // Clear buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Create view and projection matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // Draw skybox first
        glDepthFunc(GL_LEQUAL);
        glUseProgram(skyboxShader);

        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // Use sphere shader and set uniform values
        glUseProgram(sphereShaderProgram);

        glUniform3fv(glGetUniformLocation(sphereShaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        glUniform1f(glGetUniformLocation(sphereShaderProgram, "shininess"), 32.0f);

        // Update light positions (optional - create moving lights)
        float time = glfwGetTime();
        lights[0].setPosition(glm::vec3(sinf(time) * 3.0f, cosf(time) * 2.0f, 3.0f));
        lights[1].setPosition(glm::vec3(3.0f, sinf(time * 0.7f) * 2.0f, cosf(time * 0.5f) * 3.0f));
        lights[2].setPosition(glm::vec3(-3.0f, sinf(time * 0.7f) * 2.0f, -cosf(time * 0.5f) * 3.0f));

        // Update all lights in the shader
        glUniform1i(glGetUniformLocation(sphereShaderProgram, "numLights"), lights.size());
        for (size_t i = 0; i < lights.size(); i++) {
            lights[i].updateShader(sphereShaderProgram, i);
        }

        // Render all spheres
        for (auto& sphere : spheres) {
            sphere.render(sphereShaderProgram, view, projection);
        }

        // Render light spheres
     /*   Sphere lightSphere(glm::vec3(0.0f), 0.1f, 12, 12);
        lightSphere.setup();

        for (const auto& light : lights) {
            lightSphere.setPosition(light.getPosition());
            lightSphere.setColor(light.getColor());
           lightSphere.render(sphereShaderProgram, view, projection);
        }*/

        glUseProgram(modelShaderProgram);

        //// Set gun lighting uniforms
        //glUniform3fv(glGetUniformLocation(modelShaderProgram, "objectColor"), 1, glm::value_ptr(glm::vec3(0.3f, 0.3f, 0.3f)));
        //glUniform3fv(glGetUniformLocation(modelShaderProgram, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
        //glUniform3fv(glGetUniformLocation(modelShaderProgram, "lightPos"), 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 3.0f)));
        //glUniform3fv(glGetUniformLocation(modelShaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));

        //// Calculate gun position relative to camera
        //glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
        //glm::vec3 gunOffset = cameraRight * 0.3f + cameraUp * (-0.2f) + cameraFront * 0.5f;
        //glm::vec3 gunPos = cameraPos + gunOffset;

        //gunModel.setPosition(gunPos);

        //// Use quaternion-based rotation
        //glm::quat gunRotation = calculateGunRotation(cameraFront, cameraUp);
        //gunModel.setRotationFromQuaternion(gunRotation);

        //// Draw the gun
        //gunModel.draw(modelShaderProgram, view, projection);


        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
        glm::vec3 gunOffset = cameraRight * 0.3f + cameraUp * (-0.2f) + cameraFront * 0.5f;
        glm::vec3 gunPos = cameraPos + gunOffset;

        gunModel.setPosition(gunPos);

        // Use direct camera rotation values
        glm::vec3 gunRotation = { pitch, -yaw, 90.0f };

        gunModel.setRotation(gunRotation);

        renderGunModel(modelShaderProgram, gunModel, view, projection, lights, cameraPos, cameraFront, cameraUp);
       // renderGunModel(modelShaderProgram, gunModel, view, projection, lights, cameraPos, cameraFront, cameraUp);

        // Draw crosshair (disable depth test so it's always on top)
        glDisable(GL_DEPTH_TEST);
        glUseProgram(crosshairShaderProgram);
        glBindVertexArray(crosshairVAO);
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 4);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &crosshairVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &skyboxEBO);
    glDeleteBuffers(1, &crosshairVBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(skyboxShader);
    glDeleteProgram(crosshairShaderProgram);
    glDeleteProgram(sphereShaderProgram);
    glDeleteTextures(1, &cubemapTexture);
    glDeleteProgram(modelShaderProgram);

    glfwTerminate();
    return 0;
}