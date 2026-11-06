#include "Pidgeys.h"

#include <cmath>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Donde se pega cada ala al cuerpo
static const glm::vec3 hombroIzq( 0.28f, 0.05f, 0.04f);
static const glm::vec3 hombroDer(-0.28f, 0.05f, 0.04f);

// Vuelo de cada ave de la parvada
struct Vuelo
{
	float cx, cz;       // centro del circulo
	float radio;
	float altura;
	float velocidad;    // radianes por segundo (negativa = al reves)
	float fase;         // para que no empiecen en el mismo punto
	float escala;
};

static const int numAves = 8;
static const Vuelo vuelos[numAves] = {
	//   cx      cz    radio  altura  vel    fase  escala
	{ -110.0f, -40.0f, 35.0f, 28.0f,  0.20f, 0.0f, 1.0f },   // sobre la aldea
	{ -110.0f, -40.0f, 35.0f, 30.0f,  0.20f, 0.6f, 0.9f },
	{ -110.0f, -40.0f, 35.0f, 27.0f,  0.20f, 1.2f, 1.1f },
	{    0.0f,  70.0f, 60.0f, 35.0f, -0.15f, 0.0f, 1.0f },   // sobre el lago (alto)
	{    0.0f,  70.0f, 60.0f, 37.0f, -0.15f, 0.5f, 1.0f },
	{  110.0f, -30.0f, 30.0f, 25.0f,  0.25f, 2.0f, 0.9f },   // zona de criaturas
	{  110.0f, -30.0f, 30.0f, 26.0f,  0.25f, 2.7f, 1.0f },
	{    0.0f, -40.0f, 45.0f, 32.0f, -0.18f, 3.0f, 1.1f },   // entrada
};

static const float tiempoReaparecer = 8.0f;   // segundos para que salga otra presa

void Pidgeys::Crear()
{
	Cuerpo_M.LoadModel("assets/planeta_azul/modelos/Pidgey_Cuerpo.obj");
	AlaIzq_M.LoadModel("assets/planeta_azul/modelos/Pidgey_AlaIzq.obj");
	AlaDer_M.LoadModel("assets/planeta_azul/modelos/Pidgey_AlaDer.obj");
}

void Pidgeys::Actualizar(float dt)
{
	tiempo += dt;
	anguloPresa += velocidadPresa * dt;

	// Si se la comieron, despues de unos segundos aparece otra
	if (!presaViva)
	{
		tiempoComida += dt;
		if (tiempoComida > tiempoReaparecer)
			presaViva = true;
	}
}

glm::vec3 Pidgeys::PosicionPresa() const
{
	return glm::vec3(centroPresa.x + radioPresa * cosf(anguloPresa),
	                 alturaPresa,
	                 centroPresa.y + radioPresa * sinf(anguloPresa));
}

void Pidgeys::ComerPresa()
{
	presaViva = false;
	tiempoComida = 0.0f;
}

void Pidgeys::DibujarAve(const ContextoRender& ctx, glm::vec3 pos, float giro, float alabeo, float aleteo, float escala)
{
	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	//Cuerpo
	model = glm::translate(model, pos);
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));     // hacia donde vuela
	model = glm::rotate(model, alabeo, glm::vec3(0.0f, 0.0f, 1.0f));   // se inclina en la vuelta
	model = glm::scale(model, glm::vec3(escala));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	Cuerpo_M.RenderModel();

	//Ala izquierda
	model = modelaux;
	model = glm::translate(model, hombroIzq);
	model = glm::rotate(model, aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	AlaIzq_M.RenderModel();

	//Ala derecha
	model = modelaux;
	model = glm::translate(model, hombroDer);
	model = glm::rotate(model, -aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	AlaDer_M.RenderModel();
}

void Pidgeys::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	//Parvada
	for (int i = 0; i < numAves; i++)
	{
		const Vuelo& v = vuelos[i];
		float t = tiempo * v.velocidad + v.fase;

		float x = v.cx + v.radio * cosf(t);
		float z = v.cz + v.radio * sinf(t);
		float aleteo = glm::radians(40.0f) * sinf(tiempo * 9.0f + i);
		float y = v.altura + 0.3f * sinf(tiempo * 9.0f + i);              // sube un poco con cada aleteo

		// Direccion de avance = derivada del circulo 
		float s = (v.velocidad >= 0.0f) ? 1.0f : -1.0f;
		float giro = atan2f(-sinf(t) * s, cosf(t) * s);
		float alabeo = glm::radians(15.0f) * s;                             // se inclina hacia adentro

		DibujarAve(ctx, glm::vec3(x, y, z), giro, alabeo, aleteo, v.escala);
	}

	//La presa del Reaper
	if (presaViva)
	{
		glm::vec3 p = PosicionPresa();
		p.y += 0.3f * sinf(tiempo * 10.0f);
		float giro = atan2f(-sinf(anguloPresa), cosf(anguloPresa));
		float aleteo = glm::radians(40.0f) * sinf(tiempo * 10.0f);
		DibujarAve(ctx, p, giro, glm::radians(15.0f), aleteo, 1.3f);
	}
}
