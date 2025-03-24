#include <common/camera.hpp>

Camera::Camera(const glm::vec3 Eye, const glm::vec3 Target, float FovAngle) {
	eye = Eye;
	target = Target;
	fov = Maths::radians(FovAngle);
}

void Camera::calculateMatrices()
{
	calculateCameraVectors();

	// Calculate the view matrix
	view = glm::lookAt(eye, eye + front, worldUp);

	// Calculate the projection matrix
	projection = glm::perspective(fov, aspect, near, far);
}

void Camera::rotateCamera(float radius, float rotationSpeed, glm::vec3 centrePos)
{
	float modAngle = Maths::radians(glfwGetTime() * 360.0f / rotationSpeed);
	float x = centrePos.x + radius * cosf(modAngle);
	float z = centrePos.y + radius * sinf(modAngle);
	eye = glm::vec3(x, 0.0f, z);
}

void Camera::calculateCameraVectors()
{
	front = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::cross(right, front);
}


