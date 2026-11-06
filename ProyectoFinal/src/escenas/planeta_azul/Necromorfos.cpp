#include "Necromorfos.h"

#include <cmath>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

static const float escalaMixamo = 0.01f;    // Mixamo trabaja en centimetros -> metros

// La caminata de Mixamo avanza 1.75 m en 1.5 s (unos 1.17 m/s).
// Si el circulo los mueve a otra velocidad, los pies "patinan".
static const float velocidadCaminata = 1.17f;

// Que necromorfos (por numero de choza) bailan cuando se prende la fogata
static const bool bailan[] = { true, false, true, false, false, false, false };

void Necromorfos::Crear(const Terreno& t, const Aldea& aldea)
{
	terreno = &t;
	fogata = aldea.centro;

	// Malla + esqueleto + caminata (animacion 0) y despues el baile (animacion 1)
	necromorfo.Cargar("assets/planeta_azul/modelos/Necromorfo_Caminar.fbx", "assets/planeta_azul/texturas/Necromorfo.jpg");
	animCaminar = 0;
	animBailar = necromorfo.AgregarAnimacion("assets/planeta_azul/modelos/Necromorfo_Bailar.fbx");
	// La caminata no venia "In Place": la cadera avanzaba sola y al repetirse brincaba
	necromorfo.QuitarAvance(animCaminar, "mixamorig:Hips");

	shader.CreateFromFiles("shaders/shader_esqueleto.vert", "shaders/shader_light.frag");

	// Uno por choza: su circulo esta 9 m enfrente de la choza (hacia la fogata)
	numCaminantes = glm::min(aldea.NumChozas(), maxCaminantes);
	for (int i = 0; i < numCaminantes; i++)
	{
		glm::vec2 choza = aldea.PosicionChoza(i);
		glm::vec2 haciaFogata = glm::normalize(fogata - choza);

		Caminante& c = caminantes[i];
		c.centro = choza + haciaFogata * 9.0f;
		c.radio = 3.5f;
		c.angulo = i * 1.3f;                                       // cada uno empieza en otro punto
		float sentido = (i % 2 == 0) ? 1.0f : -1.0f;               // unos van al reves
		c.velocidad = sentido * velocidadCaminata / c.radio;       // v = w * r  ->  w = v / r
		c.bailaConFogata = (i < 7) ? bailan[i] : false;
	}
}

void Necromorfos::Actualizar(float dt, bool fogataEncendida)
{
	tiempo += dt;
	bailando = fogataEncendida;

	for (int i = 0; i < numCaminantes; i++)
	{
		// Los de la aldea se quedan quietos mientras bailan
		if (bailando && caminantes[i].bailaConFogata)
			continue;
		caminantes[i].angulo += caminantes[i].velocidad * dt;
	}
}

void Necromorfos::DibujarUno(const Caminante& c, int animacion, float tiempoAnimacion, GLuint uModel)
{
	// Posicion en el circulo
	float x = c.centro.x + c.radio * cosf(c.angulo);
	float z = c.centro.y + c.radio * sinf(c.angulo);

	float giro;
	if (animacion == animBailar)
	{
		giro = atan2f(fogata.x - x, fogata.y - z);              // voltea hacia la fogata
	}
	else
	{
		// Direccion de avance = derivada del circulo (con el signo de la velocidad)
		float s = (c.velocidad >= 0.0f) ? 1.0f : -1.0f;
		giro = atan2f(-sinf(c.angulo) * s, cosf(c.angulo) * s);
	}

	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(x, terreno->Altura(x, z), z));
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escalaMixamo));
	glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(model));

	necromorfo.CalcularPose(animacion, tiempoAnimacion);
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	necromorfo.Dibujar((GLuint)programa);
}

void Necromorfos::Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int numLuces)
{
	GLint shaderMain = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderMain);

	// Mismos uniforms que main.cpp manda al shader de luz
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(ctx.proyeccion));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(ctx.vista));
	glUniform3f(shader.GetEyePositionLocation(), ctx.pos_camara.x, ctx.pos_camara.y, ctx.pos_camara.z);
	shader.SetDirectionalLight(luz);
	shader.SetPointLights(luces, numLuces);
	shader.SetSpotLights(nullptr, 0);
	glUniform3f(shader.getColorLocation(), 1.0f, 1.0f, 1.0f);
	glUniform1f(shader.GetSpecularIntensityLocation(), 0.3f);
	glUniform1f(shader.GetShininessLocation(), 16.0f);

	for (int i = 0; i < numCaminantes; i++)
	{
		const Caminante& c = caminantes[i];
		bool baila = bailando && c.bailaConFogata;
		// Cada uno con un desfase en su animacion para que no se muevan identicos
		DibujarUno(c, baila ? animBailar : animCaminar, tiempo + i * 0.4f, shader.GetModelLocation());
	}

	glUseProgram(shaderMain);
}
