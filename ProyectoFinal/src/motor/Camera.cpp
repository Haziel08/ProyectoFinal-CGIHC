//>>>>>>>>>>>>>>>>>>>>>> Camera >>>>>>>>>>>>>>>>>>>>>>
#include "Camera.h"

Camera::Camera() {}

Camera::Camera(glm::vec3 start_position, glm::vec3 start_up, GLfloat start_yaw, GLfloat start_pitch, GLfloat start_move_speed, GLfloat start_turn_speed)
{
	position = start_position;
	world_up = start_up;
	yaw = start_yaw;
	pitch = start_pitch;
	front = glm::vec3(0.0f, 0.0f, -1.0f);

	move_speed = start_move_speed;
	turn_speed = start_turn_speed;

	update();
}

void Camera::keyControl(bool* keys, GLfloat delta_time)
{
	GLfloat velocity = move_speed * delta_time;

	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		position += front * velocity;
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		position -= front * velocity;
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		position -= right * velocity;
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		position += right * velocity;
	}
}

void Camera::mouseControl(GLfloat x_change, GLfloat y_change)
{
	x_change *= turn_speed;
	y_change *= turn_speed;

	yaw += x_change;
	pitch += y_change;

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

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

glm::vec3 Camera::getCameraPosition()
{
	return position;
}


glm::vec3 Camera::getCameraDirection()
{
	return glm::normalize(front);
}

void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, world_up));
	up = glm::normalize(glm::cross(right, front));
}


Camera::~Camera()
{
}

