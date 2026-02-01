#include "Light.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

Light::Light(const glm::vec3& position, const glm::vec3& color,
    float ambient, float diffuse, float specular)
    : position(position), color(color),
    ambient(ambient), diffuse(diffuse), specular(specular),
    constant(1.0f), linear(0.09f), quadratic(0.032f) {
}

void Light::updateShader(unsigned int shaderProgram, int lightIndex) {
    // Create uniform name with array index
    std::string index = std::to_string(lightIndex);

    // Update light properties in shader
    glUniform3fv(glGetUniformLocation(shaderProgram, ("lights[" + index + "].position").c_str()),
        1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(shaderProgram, ("lights[" + index + "].color").c_str()),
        1, glm::value_ptr(color));

    glUniform1f(glGetUniformLocation(shaderProgram, ("lights[" + index + "].ambient").c_str()),
        ambient);
    glUniform1f(glGetUniformLocation(shaderProgram, ("lights[" + index + "].diffuse").c_str()),
        diffuse);
    glUniform1f(glGetUniformLocation(shaderProgram, ("lights[" + index + "].specular").c_str()),
        specular);

    glUniform1f(glGetUniformLocation(shaderProgram, ("lights[" + index + "].constant").c_str()),
        constant);
    glUniform1f(glGetUniformLocation(shaderProgram, ("lights[" + index + "].linear").c_str()),
        linear);
    glUniform1f(glGetUniformLocation(shaderProgram, ("lights[" + index + "].quadratic").c_str()),
        quadratic);
}

void Light::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
}

void Light::setColor(const glm::vec3& newColor) {
    color = newColor;
}

void Light::setAmbient(float value) {
    ambient = value;
}

void Light::setDiffuse(float value) {
    diffuse = value;
}

void Light::setSpecular(float value) {
    specular = value;
}

void Light::setAttenuation(float newConstant, float newLinear, float newQuadratic) {
    constant = newConstant;
    linear = newLinear;
    quadratic = newQuadratic;
}

glm::vec3 Light::getPosition() const {
    return position;
}

glm::vec3 Light::getColor() const {
    return color;
}

float Light::getAmbient() const {
    return ambient;
}

float Light::getDiffuse() const {
    return diffuse;
}

float Light::getSpecular() const {
    return specular;
}