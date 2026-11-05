#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Camera >>>>>>>>>>>>>>>>>>>>>>

#include <glew.h>

#include <glm.hpp>
#include <cmath>
#include <gtc/matrix_transform.hpp>

#include <glfw3.h>

class Camera
{
public:
	Camera();
	Camera(glm::vec3 start_position, glm::vec3 start_up, GLfloat start_yaw, GLfloat start_pitch, GLfloat start_move_speed, GLfloat start_turn_speed);

	void keyControl(bool* keys, GLfloat delta_time);
	void mouseControl(GLfloat x_change, GLfloat y_change);

	glm::vec3 getCameraPosition();
	glm::vec3 getCameraDirection();
	glm::mat4 calculateViewMatrix();

	void setPosition(glm::vec3 p) { position = p; }
	void setMoveSpeed(GLfloat s) { move_speed = s; }

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat move_speed;
	GLfloat turn_speed;

	void update();
};

