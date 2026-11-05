#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de PointLight >>>>>>>>>>>>>>>>>>>>>>
#include "Light.h"

class PointLight :
	public Light
{
public:
	PointLight();
	PointLight(GLfloat red, GLfloat green, GLfloat blue,
		GLfloat a_intensity, GLfloat d_intensity,
		GLfloat x_pos, GLfloat y_pos, GLfloat z_pos,
		GLfloat con, GLfloat lin, GLfloat exp);

	void UseLight(GLfloat ambient_intensity_location, GLfloat ambientcolor_location,
		GLfloat diffuse_intensity_location, GLfloat position_location,
		GLfloat constant_location, GLfloat linear_location, GLfloat exponent_location);

	~PointLight();

protected:
	glm::vec3 position;

	GLfloat constant, linear, exponent;
};

