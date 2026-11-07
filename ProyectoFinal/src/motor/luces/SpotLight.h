#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de SpotLight >>>>>>>>>>>>>>>>>>>>>>
#include "PointLight.h"
class SpotLight :
	public PointLight
{
public:
	SpotLight();

	SpotLight(GLfloat red, GLfloat green, GLfloat blue,
		GLfloat a_intensity, GLfloat d_intensity,
		GLfloat x_pos, GLfloat y_pos, GLfloat z_pos,
		GLfloat x_dir, GLfloat y_dir, GLfloat z_dir,
		GLfloat con, GLfloat lin, GLfloat exp,
		GLfloat edg);

	void UseLight(GLint ambient_intensity_location, GLint ambient_colour_location,
		GLint diffuse_intensity_location, GLint position_location, GLint direction_location,
		GLint constant_location, GLint linear_location, GLint exponent_location,
		GLint edge_location);

	void SetFlash(glm::vec3 pos, glm::vec3 dir);
	void SetPos(glm::vec3 pos);
	~SpotLight();

private:
	glm::vec3 direction;

	GLfloat edge, proc_edge;
};

