#include "Reaper.h"

#include <cmath>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Articulaciones respecto a la cabeza
static const glm::vec3 pivBoca      ( 0.00f, -0.18f,  -0.68f);
static const glm::vec3 pivTentSupIzq( 0.89f,  0.15f,  -0.39f);
static const glm::vec3 pivTentSupDer(-0.89f,  0.13f,  -0.50f);
static const glm::vec3 pivTentInfIzq( 0.90f, -0.46f,  -0.35f);
static const glm::vec3 pivTentInfDer(-0.91f, -0.41f,  -0.46f);
static const glm::vec3 pivAletaIzq  ( 0.43f, -0.87f,  -2.10f);
static const glm::vec3 pivAletaDer  (-0.42f, -0.84f,  -2.20f);
static const glm::vec3 pivCola      (-0.05f,  0.48f, -17.01f);

static const float yBase = -10.0f;          // profundidad cuando nada
static const float duracionSalto = 3.5f;    // segundos
static const float escala = 1.0f;           // el modelo ya mide

void Reaper::Crear()
{
	Cuerpo_M.LoadModel("assets/planeta_azul/modelos/Reaper_Cuerpo.obj");
	Boca_M.LoadModel("assets/planeta_azul/modelos/Reaper_Boca.obj");
	TentSupIzq_M.LoadModel("assets/planeta_azul/modelos/Reaper_TentSupIzq.obj");
	TentSupDer_M.LoadModel("assets/planeta_azul/modelos/Reaper_TentSupDer.obj");
	TentInfIzq_M.LoadModel("assets/planeta_azul/modelos/Reaper_TentInfIzq.obj");
	TentInfDer_M.LoadModel("assets/planeta_azul/modelos/Reaper_TentInfDer.obj");
	AletaIzq_M.LoadModel("assets/planeta_azul/modelos/Reaper_AletaIzq.obj");
	AletaDer_M.LoadModel("assets/planeta_azul/modelos/Reaper_AletaDer.obj");
	Cola_M.LoadModel("assets/planeta_azul/modelos/Reaper_Cola.obj");
}

void Reaper::Actualizar(float dt, Window& ventana, Pidgeys& pidgeys)
{
	tiempo += dt;

	// Tecla G: salta (solo si no esta saltando y hay presa volando)
	if (ventana.TeclaPresionada(GLFW_KEY_G) && !saltando && pidgeys.PresaViva())
	{
		saltando = true;
		s = 0.0f;
		yaMordio = false;
	}

	// Siempre nada justo debajo de la presa (mismo circulo, mismo angulo)
	float a = pidgeys.AnguloPresa();
	posicion.x = pidgeys.centroPresa.x + pidgeys.radioPresa * cosf(a);
	posicion.z = pidgeys.centroPresa.y + pidgeys.radioPresa * sinf(a);
	giro = atan2f(-sinf(a), cosf(a));                 // tangente del circulo

	float velHorizontal = pidgeys.velocidadPresa * pidgeys.radioPresa;   // v = w * r
	float velVertical = 0.0f;

	if (saltando)
	{
		s += dt / duracionSalto;
		float H = pidgeys.alturaPresa - yBase;        // cuanto tiene que subir

		// Parabola: sube, llega a la presa en s = 0.5 y baja
		posicion.y = yBase + H * 4.0f * s * (1.0f - s);
		velVertical = H * 4.0f * (1.0f - 2.0f * s) / duracionSalto;   // derivada de la parabola

		// Abre la boca mientras sube y la cierra de golpe al llegar arriba
		abreBoca = (s < 0.5f) ? sinf(s * 3.1416f) : 0.0f;

		if (s >= 0.5f && !yaMordio)
		{
			pidgeys.ComerPresa();
			yaMordio = true;
		}
		if (s >= 1.0f)
		{
			saltando = false;
			posicion.y = yBase;
		}
	}
	else
	{
		posicion.y = yBase + 0.6f * sinf(tiempo * 0.8f);
		abreBoca = 0.15f + 0.15f * sinf(tiempo * 1.5f);   // la abre poquito mientras nada
	}

	// Inclinacion del cuerpo: angulo de la velocidad (vertical contra horizontal).
	// En el salto se usa directo al caer al agua regresa poco a poco a horizontal.
	float inclinacionObjetivo = atan2f(velVertical, velHorizontal);
	if (saltando)
		inclinacion = inclinacionObjetivo;
	else
		inclinacion += (inclinacionObjetivo - inclinacion) * glm::min(1.0f, 3.0f * dt);
}

void Reaper::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// Angulos de la animacion
	float coleteo = glm::radians(12.0f) * sinf(tiempo * 2.0f);
	float aleteo = glm::radians(15.0f) * sinf(tiempo * 2.5f);
	float mandibula = glm::radians(30.0f) * abreBoca;
	// Los tentaculos se mueven solos y se abren mucho al atacar
	float tentaculos = glm::radians(8.0f) * sinf(tiempo * 3.0f) + glm::radians(35.0f) * abreBoca;

	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	//Cuerpo
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -inclinacion, glm::vec3(1.0f, 0.0f, 0.0f));   // nariz arriba/abajo
	model = glm::scale(model, glm::vec3(escala));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	Cuerpo_M.RenderModel();

	//Boca
	model = modelaux;
	model = glm::translate(model, pivBoca);
	model = glm::rotate(model, mandibula, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	Boca_M.RenderModel();

	//Tentaculos de la cara
	model = modelaux;
	model = glm::translate(model, pivTentSupIzq);
	model = glm::rotate(model, tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	TentSupIzq_M.RenderModel();

	model = modelaux;
	model = glm::translate(model, pivTentSupDer);
	model = glm::rotate(model, -tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	TentSupDer_M.RenderModel();

	model = modelaux;
	model = glm::translate(model, pivTentInfIzq);
	model = glm::rotate(model, tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	TentInfIzq_M.RenderModel();

	model = modelaux;
	model = glm::translate(model, pivTentInfDer);
	model = glm::rotate(model, -tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	TentInfDer_M.RenderModel();

	//Aletas
	model = modelaux;
	model = glm::translate(model, pivAletaIzq);
	model = glm::rotate(model, aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	AletaIzq_M.RenderModel();

	model = modelaux;
	model = glm::translate(model, pivAletaDer);
	model = glm::rotate(model, -aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	AletaDer_M.RenderModel();

	//Cola
	model = modelaux;
	model = glm::translate(model, pivCola);
	model = glm::rotate(model, coleteo, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	Cola_M.RenderModel();
}
