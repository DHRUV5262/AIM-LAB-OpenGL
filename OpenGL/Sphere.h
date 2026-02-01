#ifndef SPHERE_H
#define SPHERE_H

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

class Sphere {
public:
    // Constructor with parameters for position, radius, and detail level
    Sphere(const glm::vec3& position = glm::vec3(0.0f),
        float radius = 1.0f,
        unsigned int sectors = 36,
        unsigned int stacks = 18);

    // Destructor to clean up OpenGL resources
    ~Sphere();

    // Setup method to initialize the sphere's VAO, VBO, and EBO
    void setup();

    // Render method to draw the sphere
    void render(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection);

    // Getters and setters for sphere properties
    void setPosition(const glm::vec3& newPosition);
    void setRadius(float newRadius);
    void setColor(const glm::vec3& newColor);

    glm::vec3 getPosition() const;
    float getRadius() const;
    glm::vec3 getColor() const;
    

private:
    // Method to generate vertices and indices for sphere
    void generateVertices();

    // Sphere properties
    glm::vec3 position;
    float radius;
    unsigned int sectors;  // Longitude divisions
    unsigned int stacks;   // Latitude divisions
    glm::vec3 color;

    // OpenGL objects
    unsigned int VAO, VBO, EBO;

    // Mesh data
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    bool isSetup;
};

#endif // SPHERE_H