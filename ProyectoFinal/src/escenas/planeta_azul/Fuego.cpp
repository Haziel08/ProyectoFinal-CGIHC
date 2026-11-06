#include "Fuego.h"

#include <vector>
#include <gtc/matrix_transform.hpp>

Fuego::~Fuego()
{
	delete malla;
}

void Fuego::Crear()
{
	// Dos cuadros cruzados de 1 x 1 (uno en el plano XY y otro en el ZY),
	// con varios cuadritos por lado para que las ondas los puedan doblar.
	const int div = 6;
	std::vector<GLfloat> v;
	std::vector<unsigned int> idx;
	for (int cuadro = 0; cuadro < 2; cuadro++)
	{
		unsigned int b = (unsigned int)(v.size() / 8);
		for (int i = 0; i <= div; i++)          // a lo ancho
		{
			for (int j = 0; j <= div; j++)      // a lo alto
			{
				float u = (float)i / div, w = (float)j / div;
				float x = (cuadro == 0) ? u - 0.5f : 0.0f;
				float z = (cuadro == 0) ? 0.0f : u - 0.5f;
				v.insert(v.end(), { x, w, z,  u, w,  0.0f, 0.0f, 1.0f });
			}
		}
		for (int i = 0; i < div; i++)
		{
			for (int j = 0; j < div; j++)
			{
				unsigned int a = b + i * (div + 1) + j, c = a + div + 1;
				idx.insert(idx.end(), { a, c, a + 1,  a + 1, c, c + 1 });
			}
		}
	}
	malla = new MeshModel();
	malla->CreateMeshModel(v.data(), idx.data(), (unsigned int)v.size(), (unsigned int)idx.size());

	textura = Texture("assets/planeta_azul/texturas/fuego.jpg");
	textura.LoadTextureA();
	shader.Crear("shaders/shader_animado.vert", "shaders/shader_animado.frag");
}

void Fuego::Dibujar(const ContextoRender& ctx, glm::vec3 posicion, float ancho, float alto, float tiempo)
{
	GLint shaderAnterior = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderAnterior);

	glm::mat4 model(1.0f);
	model = glm::translate(model, posicion);
	model = glm::scale(model, glm::vec3(ancho, alto, ancho));

	shader.Usar();
	shader.SetMat4("model", model);
	shader.SetMat4("view", ctx.vista);
	shader.SetMat4("projection", ctx.proyeccion);

	// Movimiento la llama tiembla rapido y la textura sube
	shader.SetFloat("tiempo", tiempo + posicion.x);   // cada fuego desfasado
	shader.SetFloat("amplitud", 0.08f);
	shader.SetFloat("frecuencia", 6.0f);
	shader.SetFloat("velocidad", 10.0f);
	shader.SetVec2("velocidadUV", glm::vec2(0.0f, -0.8f));   // la textura sube
	shader.SetFloat("repeticionUV", 1.0f);

	// Aspecto sin luz
	shader.SetVec3("colorBase", glm::vec3(1.0f));
	shader.SetFloat("transparencia", 1.0f);
	shader.SetFloat("desvanecer", 1.0f);
	shader.SetInt("usarLuz", 0);

	// Mezcla
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);

	textura.UseTexture();
	malla->RenderMeshModel();

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glUseProgram(shaderAnterior);
}
