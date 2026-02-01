#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <string>


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int VAO, VBO, EBO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
    void draw(unsigned int shaderProgram);
    void cleanup();

private:
    void setupMesh();
};

class Model {
private:
    std::vector<Mesh> meshes;
    glm::vec3 position;
    glm::vec3 rotation;    // Euler angles (degrees) - kept for compatibility
    glm::vec3 scale;

    // Quaternion support
    glm::quat rotationQuat;
    bool useQuaternion;

public:
    Model(const std::string& path);
    ~Model();

    void draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection);

    // Transform setters/getters
    void setPosition(const glm::vec3& pos) { position = pos; }
    void setRotation(const glm::vec3& rot) { rotation = rot; useQuaternion = false; }
    void setScale(const glm::vec3& scl) { scale = scl; }

    // Quaternion methods
    void setRotationFromQuaternion(const glm::quat& quat);
       

    void enableQuaternionRotation(bool enable) { useQuaternion = enable; }

    glm::vec3 getPosition() const { return position; }
    glm::vec3 getRotation() const { return rotation; }
    glm::vec3 getScale() const { return scale; }
    glm::quat getRotationQuaternion() const { return rotationQuat; }


private:
    void loadModel(const std::string& path);
    glm::mat4 getModelMatrix() const;
};