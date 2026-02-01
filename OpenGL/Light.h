#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <string>

class Light {
public:
    // Constructor
    Light(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        float ambient = 0.1f,
        float diffuse = 0.8f,
        float specular = 1.0f);

    // Update light parameters in shader
    void updateShader(unsigned int shaderProgram, int lightIndex);

    // Getters and setters
    void setPosition(const glm::vec3& newPosition);
    void setColor(const glm::vec3& newColor);
    void setAmbient(float value);
    void setDiffuse(float value);
    void setSpecular(float value);
    void setAttenuation(float constant, float linear, float quadratic);

    float getAmbient() const;
    float getDiffuse() const;
    float getSpecular() const;

    glm::vec3 getPosition() const;
    glm::vec3 getColor() const;

private:
    // Light properties
    glm::vec3 position;
    glm::vec3 color;

    // Light intensity components
    float ambient;
    float diffuse;
    float specular;

    // Attenuation factors
    float constant;
    float linear;
    float quadratic;
};

#endif // LIGHT_H