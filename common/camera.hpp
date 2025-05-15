#pragma once

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <common/maths.hpp>

class Camera
{
public:
    // Projection parameters
    float fov;
    float aspect = 1024.0f / 768.0f;
    float near = 0.1f;
    float far = 100.0f;

    // Camera vectors
    glm::vec3 eye;
    glm::vec3 target;
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Transformation matrices
    glm::mat4 view;
    glm::mat4 projection;

    // Movement vectors
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);

    // Movement control
    bool isJumping = false;
    bool isThird = false;
    bool isFreeCam = false;

    // Camera Euler angles
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    // Quaternion camera
    Quaternion orientation = Quaternion(pitch, yaw);

    // Constructor
    Camera(const glm::vec3 eye, const glm::vec3 target);

    // Methods
    void calculateMatrices();
    void rotateCamera(float radius, float rotationSpeed, glm::vec3 centrePos); // Simple function to rotate the camera around a position
    void calculateCameraVectors();
    void quaternionCamera();

private:
    glm::mat4 calculateView(glm::vec3 eye, glm::vec3 target, glm::vec3 worldUp);
    glm::mat4 calculatePerspective(float fov, float aspect, float near, float far);
};