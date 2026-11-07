//>>>>>>>>>>>>>>>>>>>>>> Material >>>>>>>>>>>>>>>>>>>>>>
#include "Material.h"



Material::Material()
{
	specular_intensity = 0.0f;
	shininess = 0.0f;
}
	
Material::Material(GLfloat s_intensity, GLfloat shine)
{
	specular_intensity = s_intensity;
	shininess = shine;
}

void Material::UseMaterial(GLuint specular_intensity_location, GLuint shininess_location)
{
	glUniform1f(specular_intensity_location, specular_intensity);
	glUniform1f(shininess_location, shininess);
}

Material::~Material()
{
}
