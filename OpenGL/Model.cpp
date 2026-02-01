#define GLM_ENABLE_EXPERIMENTAL
#include "Model.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include "Light.h"

// Mesh implementation
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices)
    : vertices(vertices), indices(indices) {
    setupMesh();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

void Mesh::draw(unsigned int shaderProgram) {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// Model implementation
Model::Model(const std::string& path) : position(0.0f), rotation(0.0f), scale(1.0f) {
    loadModel(path);
}

Model::~Model() {
    for (auto& mesh : meshes) {
        mesh.cleanup();
    }
}

void Model::loadModel(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open model file: " << path << std::endl;
        return;
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<Vertex> finalVertices;
    std::vector<unsigned int> indices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            // Vertex position
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (prefix == "vn") {
            // Vertex normal
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (prefix == "vt") {
            // Texture coordinate
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            texCoords.push_back(texCoord);
        }
        else if (prefix == "f") {
            // Face - parse indices properly
            std::string vertex1, vertex2, vertex3;
            iss >> vertex1 >> vertex2 >> vertex3;

            // Parse each vertex of the face
            for (const std::string& vertexData : { vertex1, vertex2, vertex3 }) {
                Vertex vertex;
                std::istringstream viss(vertexData);
                std::string indexStr;

                // Parse vertex index (v/vt/vn format)
                std::getline(viss, indexStr, '/');
                int vIndex = std::stoi(indexStr) - 1; // OBJ is 1-indexed
                if (vIndex >= 0 && vIndex < vertices.size()) {
                    vertex.position = vertices[vIndex];
                }
                else {
                    vertex.position = glm::vec3(0.0f);
                }

                // Parse texture coordinate index
                if (std::getline(viss, indexStr, '/') && !indexStr.empty()) {
                    int vtIndex = std::stoi(indexStr) - 1;
                    if (vtIndex >= 0 && vtIndex < texCoords.size()) {
                        vertex.texCoords = texCoords[vtIndex];
                    }
                    else {
                        vertex.texCoords = glm::vec2(0.0f);
                    }
                }
                else {
                    vertex.texCoords = glm::vec2(0.0f);
                }

                // Parse normal index
                if (std::getline(viss, indexStr) && !indexStr.empty()) {
                    int vnIndex = std::stoi(indexStr) - 1;
                    if (vnIndex >= 0 && vnIndex < normals.size()) {
                        vertex.normal = normals[vnIndex];
                    }
                    else {
                        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }
                }
                else {
                    vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                finalVertices.push_back(vertex);
                indices.push_back(finalVertices.size() - 1);
            }
        }
    }

    file.close();

    std::cout << "Loaded model with:" << std::endl;
    std::cout << "  Original vertices: " << vertices.size() << std::endl;
    std::cout << "  Original normals: " << normals.size() << std::endl;
    std::cout << "  Original texture coords: " << texCoords.size() << std::endl;
    std::cout << "  Final vertices: " << finalVertices.size() << std::endl;
    std::cout << "  Indices: " << indices.size() << std::endl;

    // Create the mesh
    if (!finalVertices.empty() && !indices.empty()) {
        meshes.push_back(Mesh(finalVertices, indices));
    }
    else {
        std::cerr << "Warning: No valid mesh data loaded from " << path << std::endl;
    }
}

void Model::draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);

    // Set matrices
    glm::mat4 model = getModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Draw all meshes
    for (auto& mesh : meshes) {
        mesh.draw(shaderProgram);
    }
}

 

glm::mat4 Model::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);

   
    model = glm::translate(model, position);

    // Use quaternion rotation if available, otherwise fall back to Euler
    if (useQuaternion) {
        glm::mat4 rotMatrix = glm::mat4_cast(rotationQuat);
        model = model * rotMatrix;
    }
    else {
        // Original Euler rotation (keep as fallback)
        glm::quat qPitch = glm::angleAxis(glm::radians(rotation.x), glm::vec3(0, 0, 1));
        glm::quat qYaw = glm::angleAxis(glm::radians(rotation.y), glm::vec3(0, 1, 0));
        glm::quat qRoll = glm::angleAxis(glm::radians(rotation.z), glm::vec3(1, 0, 0));

        glm::quat q = qYaw * qPitch * qRoll;
        glm::mat4 rotMatrix = glm::mat4_cast(q);
        model = model * rotMatrix;
    }

    model = glm::scale(model, scale);

    return model;
}

void Model::setRotationFromQuaternion(const glm::quat& quat) {
    // Convert quaternion to Euler angles if needed for debugging
    glm::vec3 eulerAngles = glm::eulerAngles(quat);
    rotation = glm::degrees(eulerAngles);

    // Store quaternion for proper rotation calculation
    rotationQuat = quat;
}
