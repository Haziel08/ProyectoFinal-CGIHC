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

	void UseLight(GLint ambient_intensity_location, GLint ambientcolor_location,
		GLint diffuse_intensity_location, GLint position_location,
		GLint constant_location, GLint linear_location, GLint exponent_location);

	~PointLight();

protected:
	glm::vec3 position;

	GLfloat constant, linear, exponent;
};

