#include "Piso.h"

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

void Piso::Crear(const char* rutaTextura)
{
	float m = tamano / 2.0f;
	float y0 = -grosor;
	float r = tamano / metrosPorTextura;   // veces que se repite la textura a lo largo
	float g = grosor / metrosPorTextura;   // en los lados

	// x, y, z,   u, v,   nx, ny, nz  (normales invertidas como en shader_light)
	GLfloat vertices[] = {
		// arriba
		-m, 0.0f, -m,   0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
		 m, 0.0f, -m,   r,    0.0f,   0.0f, -1.0f, 0.0f,
		 m, 0.0f,  m,   r,    r,      0.0f, -1.0f, 0.0f,
		-m, 0.0f,  m,   0.0f, r,      0.0f, -1.0f, 0.0f,
		// abajo
		-m, y0, -m,     0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
		 m, y0, -m,     r,    0.0f,   0.0f, 1.0f, 0.0f,
		 m, y0,  m,     r,    r,      0.0f, 1.0f, 0.0f,
		-m, y0,  m,     0.0f, r,      0.0f, 1.0f, 0.0f,
		// frente (+Z)
		-m, y0,   m,    0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
		 m, y0,   m,    r,    0.0f,   0.0f, 0.0f, -1.0f,
		 m, 0.0f, m,    r,    g,      0.0f, 0.0f, -1.0f,
		-m, 0.0f, m,    0.0f, g,      0.0f, 0.0f, -1.0f,
		// atras (-Z)
		-m, y0,   -m,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
		 m, y0,   -m,   r,    0.0f,   0.0f, 0.0f, 1.0f,
		 m, 0.0f, -m,   r,    g,      0.0f, 0.0f, 1.0f,
		-m, 0.0f, -m,   0.0f, g,      0.0f, 0.0f, 1.0f,
		// derecha (+X)
		 m, y0,   -m,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,
		 m, y0,    m,   r,    0.0f,   -1.0f, 0.0f, 0.0f,
		 m, 0.0f,  m,   r,    g,      -1.0f, 0.0f, 0.0f,
		 m, 0.0f, -m,   0.0f, g,      -1.0f, 0.0f, 0.0f,
		// izquierda (-X)
		-m, y0,   -m,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
		-m, y0,    m,   r,    0.0f,   1.0f, 0.0f, 0.0f,
		-m, 0.0f,  m,   r,    g,      1.0f, 0.0f, 0.0f,
		-m, 0.0f, -m,   0.0f, g,      1.0f, 0.0f, 0.0f,
	};

	unsigned int indices[] = {
		0, 1, 2,    2, 3, 0,
		4, 5, 6,    6, 7, 4,
		8, 9, 10,   10, 11, 8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};

	cubo = new MeshModel();
	cubo->CreateMeshModel(vertices, indices, 192, 36);

	textura = Texture(rutaTextura);
	textura.LoadTextureA();
	// Mipmaps para que la textura no "tiemble" a lo lejos
	textura.UseTexture();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void Piso::Dibujar(const ContextoRender& ctx)
{
	glm::mat4 model(1.0f);
	glm::vec3 blanco(1.0f);
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));
	material.UseMaterial(ctx.u_specular_intensity, ctx.u_shininess);

	textura.UseTexture();
	cubo->RenderMeshModel();
}

Piso::~Piso()
{
	delete cubo;
}
