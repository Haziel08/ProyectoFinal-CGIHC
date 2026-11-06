//>>>>>>>>>>>>>>>>>>>>>> Skybox >>>>>>>>>>>>>>>>>>>>>>
#include "Skybox.h"



Skybox::Skybox()
{
}

Skybox::Skybox(std::vector<std::string> face_locations)
{
	transicion_natural = !face_locations.empty() && face_locations[0].find("planeta_naranja") != std::string::npos;
	sky_shader = new Shader();
	sky_shader->CreateFromFiles("shaders/skybox.vert", "shaders/skybox.frag");
	uniform_projection = sky_shader->GetProjectionLocation();
	uniform_view = sky_shader->GetViewLocation();

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	int width, height, bit_depth;
	if (transicion_natural) stbi_set_flip_vertically_on_load(true);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	
	for (size_t i = 0; i < 6; i++)
	{

		unsigned char *tex_data = stbi_load(face_locations[i].c_str(), &width, &height, &bit_depth, STBI_rgb);
		if (!tex_data)
		{
			printf("No se encontró : %s\n", face_locations[i].c_str());
			return;
		}
		//para cambiar el origen a la esquina inferior izquierda como necesitamos
		//stbi_set_flip_vertically_on_load(true);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data); //SIN CANAL ALPHA A ENOS QUE QUERAMOS AGREGAR EFECTO DE PARALLAX
		stbi_image_free(tex_data); //para liberar la información de la imagen
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Creando el Mesh del skybox
	unsigned int skybox_indices[] = {
		// front
		0, 1, 2,
		2, 1, 3,
		// right
		2, 3, 5,
		5, 3, 7,
		// back
		5, 7, 4,
		4, 7, 6,
		// left
		4, 6, 0,
		0, 6, 1,
		// top
		4, 0, 5,
		5, 0, 2,
		// bottom
		1, 6, 3,
		3, 6, 7
	};

	float skybox_vertices[] = {
		-1.0f, 1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//v0
		-1.0f, -1.0f, -1.0f,	0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V1
		1.0f, 1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V2
		1.0f, -1.0f, -1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V3
		-1.0f, 1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V4
		1.0f, 1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V5
		-1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f,//V6
		1.0f, -1.0f, 1.0f,		0.0f, 0.0f,		0.0f, 0.0f, 0.0f//V7
	};
	sky_mesh = new MeshModel();
	sky_mesh->CreateMeshModel(skybox_vertices, skybox_indices, 64, 36);

}

void Skybox::DrawSkybox(glm::mat4 view_matrix, glm::mat4 projection_matrix)
{
	view_matrix = glm::mat4(glm::mat3(view_matrix));

	GLint funcion_profundidad = GL_LESS;
    glGetIntegerv(GL_DEPTH_FUNC, &funcion_profundidad);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(false);
	sky_shader->UseShader();
	GLint programa_skybox = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa_skybox);
	glUniform1i(glGetUniformLocation(programa_skybox, "transicion_natural"), transicion_natural);
	glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(uniform_view, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
	//sky_shader->Validate();
	sky_mesh->RenderMeshModel();
	glDepthMask(true);
    glDepthFunc(funcion_profundidad);

}


Skybox::~Skybox()
{
}

