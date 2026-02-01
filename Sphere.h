#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Sphere {
public:
    Sphere(float radius, int sectorCount, int stackCount);
    ~Sphere();
    void draw(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);

private:
    void createSphere(float radius, int sectorCount, int stackCount);
    GLuint VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int indexCount;
};
