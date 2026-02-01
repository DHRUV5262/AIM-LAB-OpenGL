#include "Sphere.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Sphere::Sphere(const glm::vec3& position, float radius, unsigned int sectors, unsigned int stacks)
    : position(position), radius(radius), sectors(sectors), stacks(stacks),
    color(glm::vec3(1.0f)), isSetup(false), VAO(0), VBO(0), EBO(0) {
    generateVertices();
}

Sphere::~Sphere() {
    if (isSetup) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}

void Sphere::generateVertices() {
    vertices.clear();
    indices.clear();

    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;

    // Generate vertices
    for (unsigned int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * stackStep;  // starting from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);         // r * cos(u)
        float z = radius * sinf(stackAngle);          // r * sin(u)

        // Add (sectors+1) vertices per stack
        // The first and last vertices have same position and normal, but different texture coordinates
        for (unsigned int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;       // starting from 0 to 2pi

            // Vertex position
            float x = xy * cosf(sectorAngle);         // r * cos(u) * cos(v)
            float y = xy * sinf(sectorAngle);         // r * cos(u) * sin(v)

            // Normalized vertex normal
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));

            // Add position
            vertices.push_back(x + position.x);
            vertices.push_back(y + position.y);
            vertices.push_back(z + position.z);

            // Add normal
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Add color
            vertices.push_back(color.r);
            vertices.push_back(color.g);
            vertices.push_back(color.b);
        }
    }

    // Generate indices
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    unsigned int k1, k2;
    for (unsigned int i = 0; i < stacks; ++i) {
        k1 = i * (sectors + 1);
        k2 = k1 + sectors + 1;

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding the first and last stacks
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

void Sphere::setup() {
    if (isSetup) {
        // Clean up previous VAO, VBO, EBO if already set up
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    // Create and bind VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create and bind VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Create and bind EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);

    isSetup = true;
}

void Sphere::render(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    if (!isSetup) {
        setup();
    }

    glUseProgram(shaderProgram);

    // Create model matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    // Set matrices
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Draw sphere
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}



void Sphere::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
    generateVertices();  // Regenerate vertices with new position
    if (isSetup) {
        setup();  // Update the buffers
    }
}

void Sphere::setRadius(float newRadius) {
    radius = newRadius;
    generateVertices();  // Regenerate vertices with new radius
    if (isSetup) {
        setup();  // Update the buffers
    }
}

void Sphere::setColor(const glm::vec3& newColor) {
    color = newColor;
    generateVertices();  // Regenerate vertices with new color
    if (isSetup) {
        setup();  // Update the buffers
    }
}

glm::vec3 Sphere::getPosition() const {
    return position;
}

float Sphere::getRadius() const {
    return radius;
}

glm::vec3 Sphere::getColor() const {
    return color;
}