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
	view = calculateView(eye, eye + front, worldUp);

	// Calculate the projection matrix
	projection = calculatePerspective(fov, aspect, near, far);
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

// My version of glm::lookAt
glm::mat4 Camera::calculateView(glm::vec3 eye, glm::vec3 target, glm::vec3 worldUp)
{
	// Create translate matrix
	glm::mat4 translate;
	translate[0][0] = 1.0f, translate[0][3] = -eye.x;
	translate[1][1] = 1.0f, translate[1][3] = -eye.y;
	translate[2][2] = 1.0f, translate[2][3] = -eye.z;
	translate[3][3] = 1.0f;

	//// Calculate vectors
	//glm::vec3 front = (target - eye) / (glm::length(target - eye));								// Front = (target - eye) / ||(target - eye)||						
	//glm::vec3 right = (glm::cross(front, worldUp)) / (glm::length(glm::cross(front, worldUp)));	// Right = (Front X WorldUp) / ||(Front X WorldUp)||
	//glm::vec3 up = glm::cross(right, front);														// Up = Right X Front

	// Create rotation matrix
	glm::mat4 rotate;
	rotate[0][0] = right.x, rotate[0][1] = right.y, rotate[0][2] = right.z;
	rotate[1][0] = up.x, rotate[1][1] = up.y, rotate[1][2] = up.z;
	rotate[2][0] = -front.x, rotate[2][1] = -front.y, rotate[2][2] = -front.z;
	rotate[3][3] = 1.0f;

	// Calculate view matrix
	glm::mat4  view = rotate * translate;
	
	return view;
}

// My version of glm::perspective
glm::mat4 Camera::calculatePerspective(float fov, float aspect, float near, float far)
{
	// Calculate extra values needed
	float top = near * tanf(fov / 2);
	float right = aspect * top;

	// Create matrix
	glm::mat4 perspective;
	perspective[0][0] = near / right;
	perspective[1][1] = near / top;
	perspective[2][2] = -((far + near) / (far - near));
	perspective[2][3] = -1.0f;
	perspective[3][2] = -((2 * far * near) / (far - near));

	return perspective;
}


