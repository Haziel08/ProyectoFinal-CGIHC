#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Skybox >>>>>>>>>>>>>>>>>>>>>>

#include "CommonValues.h"
#include <vector>
#include <string>
#include<glew.h>
#include<glm.hpp>
#include<glfw3.h>
#include <gtc/matrix_transform.hpp>
#include<gtc/type_ptr.hpp>
#include "Mesh_tn.h"
#include "Shader_light.h"
class Skybox
{
public:
	Skybox();
	Skybox(std::vector<std::string> face_locations);
	void DrawSkybox(glm::mat4 view_matrix, glm::mat4 projection_matrix);
		
	~Skybox();
private:
	MeshModel* sky_mesh;
	Shader* sky_shader;
	GLuint texture_id;
	GLuint uniform_projection, uniform_view;
	bool transicion_natural = false;

};

