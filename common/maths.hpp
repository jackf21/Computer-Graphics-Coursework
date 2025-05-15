# pragma once

#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

#define PI 3.1415926536

// Quaternion class
class Quaternion
{
public:
    float w, x, y, z;

    // Constructors
    Quaternion();
    Quaternion(const float w, const float x, const float y, const float z);
    Quaternion(const float pitch, const float yaw);
    glm::mat4 matrix();
};

class Maths 
{
public:


    // Transformation matrices
    static glm::mat4    translate(const glm::vec3& v);
    static glm::mat4    scale(const glm::vec3& v);

    static float        radians(float angle);
    static glm::mat4    rotate(const float& angle, glm::vec3 v);

    // Vector operations
    static float        length(glm::vec3 vector);
    static glm::vec3    normalise(glm::vec3 vector);
    static float        dot(glm::vec3 a, glm::vec3 b);
    static glm::vec3    cross(glm::vec3 a, glm::vec3 b);

    // Matrix operations
    static glm::mat4    transpose(glm::mat4 matrix);

    static float clamp(float value, float min, float max);
    static float square(float value);
    static float lerp(float a, float b, float t);

    static Quaternion SLERP(const Quaternion q1, const Quaternion q2, const float t);
};
