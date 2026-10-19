#include "EscenaEspacio.h"

#include <vector>
#include <cmath>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Esfera de radio 1 con formato  x y z | u v | nx ny nz  (normales invertidas, como pide el shader)
static MeshModel* CrearEsfera(int rebanadas, int pilas)
{
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int i = 0; i <= pilas; i++)
	{
		float phi = 3.14159265f * i / pilas;
		for (int j = 0; j <= rebanadas; j++)
		{
			float theta = 2.0f * 3.14159265f * j / rebanadas;
			float x = sinf(phi) * cosf(theta), y = cosf(phi), z = sinf(phi) * sinf(theta);
			v.insert(v.end(), { x, y, z, (float)j / rebanadas, 1.0f - (float)i / pilas, -x, -y, -z });
		}
	}
	for (int i = 0; i < pilas; i++)
		for (int j = 0; j < rebanadas; j++)
		{
			unsigned int a = i * (rebanadas + 1) + j, b = a + rebanadas + 1;
			idx.insert(idx.end(), { a, b, a + 1, a + 1, b, b + 1 });
		}
	MeshModel* m = new MeshModel();
	m->CreateMeshModel(v.data(), idx.data(), (unsigned int)v.size(), (unsigned int)idx.size());
	return m;
}

void EscenaEspacio::Inicializar()
{
	CargarSkybox("assets/espacio/skybox/");

	// Cámara libre de esta escena: posición inicial, vector arriba, yaw, pitch,
	// velocidad (unidades/segundo) y sensibilidad del ratón
	camara = Camera(glm::vec3(0.0f, 5.0f, 20.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 20.0f, 0.3f);

	// Aquí se cargarán modelos, texturas y luces de la escena
	esferaPrueba = CrearEsfera(40, 40);
	texturaBlanca = Texture("assets/comun/texturas/plain.png");
	texturaBlanca.LoadTextureA();
}

void EscenaEspacio::Actualizar(float dt, Window& ventana)
{
	// Aquí van las animaciones y las teclas propias de la escena
}

void EscenaEspacio::Dibujar(const ContextoRender& ctx)
{
	// Aquí se dibujan los objetos de la escena
	// ESFERA DE PRUEBA: gigante, 60 unidades enfrente de donde inicia la cámara
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -60.0f));
	model = glm::scale(model, glm::vec3(15.0f));   // radio 15
	glm::vec3 color(0.9f, 0.5f, 0.2f);
	glUniformMatrix4fv(ctx.uModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(ctx.uColor, 1, glm::value_ptr(color));
	texturaBlanca.UseTexture();
	materialOpaco.UseMaterial(ctx.uSpecularIntensity, ctx.uShininess);
	esferaPrueba->RenderMeshModel();
}
