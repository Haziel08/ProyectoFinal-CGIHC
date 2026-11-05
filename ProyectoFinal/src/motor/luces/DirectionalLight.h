#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de DirectionalLight >>>>>>>>>>>>>>>>>>>>>>
#include "Light.h"

class DirectionalLight :
	public Light
{
public:
	DirectionalLight();
	DirectionalLight(GLfloat red, GLfloat green, GLfloat blue, 
					GLfloat a_intensity, GLfloat d_intensity,
					GLfloat x_dir, GLfloat y_dir, GLfloat z_dir);

	void UseLight(GLfloat ambient_intensity_location, GLfloat ambientcolor_location,
		GLfloat diffuse_intensity_location, GLfloat direction_location);

	~DirectionalLight();

private:
	glm::vec3 direction;
};

