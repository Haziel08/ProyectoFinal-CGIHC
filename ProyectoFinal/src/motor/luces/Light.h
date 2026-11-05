#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Light >>>>>>>>>>>>>>>>>>>>>>

#include <glew.h>
#include <glm.hpp>

class Light
{
public:
	Light();
	Light(GLfloat red, GLfloat green, GLfloat blue, 
			GLfloat a_intensity, GLfloat d_intensity);

	~Light();

protected:
	glm::vec3 color;
	GLfloat ambient_intensity;
	GLfloat diffuse_intensity;
};

