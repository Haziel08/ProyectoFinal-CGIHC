#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Material >>>>>>>>>>>>>>>>>>>>>>

#include <glew.h>
//Esta clase está pensada para crear materiales asignables a nuestros diferentes objetos y luces
class Material
{
public:
	Material();
	Material(GLfloat s_intensity, GLfloat shine);

	void UseMaterial(GLuint specular_intensity_location, GLuint shininess_location);

	~Material();

private: 
	GLfloat specular_intensity;
	GLfloat shininess;
};

