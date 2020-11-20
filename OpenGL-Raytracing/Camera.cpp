#include "Camera.h"

Camera::Camera() {}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed, float fov)
{
	position = startPosition;
	worldUp = startUp;
	yaw = startYaw;
	pitch = startPitch;
	front = glm::vec3(0.0f, 0.0f, 1.0f);

	startingMoveSpeed = startMoveSpeed;
	moveSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;

	m_fov = fov;

	update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime)
{
	GLfloat velocity = -moveSpeed * deltaTime;


	if (keys[GLFW_KEY_E]) {
		position.y += velocity;
	}

	if (keys[GLFW_KEY_Q]) {
		position.y -= velocity;
	}

	if (keys[GLFW_KEY_W])
	{
		position -= front * velocity; //old
	}

	if (keys[GLFW_KEY_S])
	{
		position += front * velocity;
	}

	if (keys[GLFW_KEY_A])
	{
		position -= right * velocity;
	}

	if (keys[GLFW_KEY_D])
	{
		position += right * velocity;
	}

	if (keys[GLFW_KEY_LEFT_SHIFT]) 
	{
		moveSpeed = 10.0f;
	}
	else
	{
		moveSpeed = startingMoveSpeed;
	}
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{
	xChange *= turnSpeed;
	yChange *= turnSpeed;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}

	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	update();
}

glm::mat4 Camera::CalculateViewMatrix()
{
	
	glm::mat4 temp = glm::lookAt(position, position + front, up);
	temp[0][3] = GetCameraPosition().x;
	temp[1][3] = GetCameraPosition().y;
	temp[2][3] = GetCameraPosition().z;

	return temp;
}

glm::mat4 Camera::CalculateProjectionMatrix(GLfloat bufferWidth, GLfloat bufferHeight)
{
	return glm::perspective(glm::radians(m_fov), bufferWidth / bufferHeight, 0.1f, 100.f);
}

glm::vec3 Camera::GetCameraPosition()
{
	return position;
}

glm::vec3 Camera::getCameraDirection()
{
	return glm::normalize(front);
}

GLfloat Camera::GetYaw()
{
	return yaw;
}

GLfloat Camera::GetPitch()
{
	return pitch;
}

void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

Camera::~Camera()
{
}
