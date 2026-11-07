#include "Pidgeys.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Pidgeys >>>>>>>>>>>>>>>>>>>>>>


// Donde se pega cada ala al cuerpo
static const glm::vec3 hombro_izq( 0.28f, 0.05f, 0.04f);
static const glm::vec3 hombro_der(-0.28f, 0.05f, 0.04f);

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

static const int num_aves = 8;
static const Vuelo vuelos[num_aves] = {
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

static const float tiempo_reaparecer = 8.0f;   // segundos para que salga otra presa

void Pidgeys::Crear()
{
	cuerpo.LoadModel("assets/planeta_azul/modelos/pidgey_cuerpo.obj");
	ala_izq.LoadModel("assets/planeta_azul/modelos/pidgey_ala_izq.obj");
	ala_der.LoadModel("assets/planeta_azul/modelos/pidgey_ala_der.obj");
}

void Pidgeys::Actualizar(float dt)
{
	tiempo += dt;
	angulo_presa += velocidad_presa * dt;

	// Si se la comieron, despues de unos segundos aparece otra
	if (!presa_viva)
	{
		tiempo_comida += dt;
		if (tiempo_comida > tiempo_reaparecer)
			presa_viva = true;
	}
}

glm::vec3 Pidgeys::PosicionPresa() const
{
	return glm::vec3(centro_presa.x + radio_presa * cosf(angulo_presa),
	                 altura_presa,
	                 centro_presa.y + radio_presa * sinf(angulo_presa));
}

void Pidgeys::ComerPresa()
{
	presa_viva = false;
	tiempo_comida = 0.0f;
}

void Pidgeys::DibujarAve(const ContextoRender& ctx, glm::vec3 pos, float giro, float alabeo, float aleteo, float escala_ave)
{
	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	//Cuerpo
	model = glm::translate(model, pos);
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));     // hacia donde vuela
	model = glm::rotate(model, alabeo, glm::vec3(0.0f, 0.0f, 1.0f));   // se inclina en la vuelta
	model = glm::scale(model, glm::vec3(escala_ave));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cuerpo.RenderModel();

	//Ala izquierda
	model = modelaux;
	model = glm::translate(model, hombro_izq);
	model = glm::rotate(model, aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	ala_izq.RenderModel();

	//Ala derecha
	model = modelaux;
	model = glm::translate(model, hombro_der);
	model = glm::rotate(model, -aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	ala_der.RenderModel();
}

void Pidgeys::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	//Parvada
	for (int i = 0; i < num_aves; i++)
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
	if (presa_viva)
	{
		glm::vec3 p = PosicionPresa();
		p.y += 0.3f * sinf(tiempo * 10.0f);
		float giro = atan2f(-sinf(angulo_presa), cosf(angulo_presa));
		float aleteo = glm::radians(40.0f) * sinf(tiempo * 10.0f);
		DibujarAve(ctx, p, giro, glm::radians(15.0f), aleteo, 1.3f);
	}
}
}
