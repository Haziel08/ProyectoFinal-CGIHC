#include "Reaper.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Reaper >>>>>>>>>>>>>>>>>>>>>>


// Articulaciones respecto a la cabeza
static const glm::vec3 piv_boca      ( 0.00f, -0.18f,  -0.68f);
static const glm::vec3 piv_tent_sup_izq( 0.89f,  0.15f,  -0.39f);
static const glm::vec3 piv_tent_sup_der(-0.89f,  0.13f,  -0.50f);
static const glm::vec3 piv_tent_inf_izq( 0.90f, -0.46f,  -0.35f);
static const glm::vec3 piv_tent_inf_der(-0.91f, -0.41f,  -0.46f);
static const glm::vec3 piv_aleta_izq  ( 0.43f, -0.87f,  -2.10f);
static const glm::vec3 piv_aleta_der  (-0.42f, -0.84f,  -2.20f);
static const glm::vec3 piv_cola      (-0.05f,  0.48f, -17.01f);

static const float y_base = -10.0f;          // profundidad cuando nada
static const float duracion_salto = 3.5f;    // segundos
static const float escala_reaper = 1.0f;           // el modelo ya mide

void Reaper::Crear()
{
	cuerpo.LoadModel("assets/planeta_azul/modelos/reaper_cuerpo.obj");
	boca.LoadModel("assets/planeta_azul/modelos/reaper_boca.obj");
	tent_sup_izq.LoadModel("assets/planeta_azul/modelos/reaper_tent_sup_izq.obj");
	tent_sup_der.LoadModel("assets/planeta_azul/modelos/reaper_tent_sup_der.obj");
	tent_inf_izq.LoadModel("assets/planeta_azul/modelos/reaper_tent_inf_izq.obj");
	tent_inf_der.LoadModel("assets/planeta_azul/modelos/reaper_tent_inf_der.obj");
	aleta_izq.LoadModel("assets/planeta_azul/modelos/reaper_aleta_izq.obj");
	aleta_der.LoadModel("assets/planeta_azul/modelos/reaper_aleta_der.obj");
	cola.LoadModel("assets/planeta_azul/modelos/reaper_cola.obj");
}

void Reaper::Actualizar(float dt, Window& ventana, Pidgeys& pidgeys)
{
	tiempo += dt;

	// Tecla U: salta (solo si no esta saltando y hay presa volando)
	if (ventana.TeclaPresionada(GLFW_KEY_U) && !saltando && pidgeys.PresaViva())
	{
		saltando = true;
		s = 0.0f;
		ya_mordio = false;
	}

	// Siempre nada justo debajo de la presa (mismo circulo, mismo angulo)
	float a = pidgeys.AnguloPresa();
	posicion.x = pidgeys.centro_presa.x + pidgeys.radio_presa * cosf(a);
	posicion.z = pidgeys.centro_presa.y + pidgeys.radio_presa * sinf(a);
	giro = atan2f(-sinf(a), cosf(a));                 // tangente del circulo

	float vel_horizontal = pidgeys.velocidad_presa * pidgeys.radio_presa;   // v = w * r
	float vel_vertical = 0.0f;

	if (saltando)
	{
		s += dt / duracion_salto;
		float H = pidgeys.altura_presa - y_base;        // cuanto tiene que subir

		// Parabola: sube, llega a la presa en s = 0.5 y baja
		posicion.y = y_base + H * 4.0f * s * (1.0f - s);
		vel_vertical = H * 4.0f * (1.0f - 2.0f * s) / duracion_salto;   // derivada de la parabola

		// Abre la boca mientras sube y la cierra de golpe al llegar arriba
		abre_boca = (s < 0.5f) ? sinf(s * 3.142f) : 0.0f;

		if (s >= 0.5f && !ya_mordio)
		{
			pidgeys.ComerPresa();
			ya_mordio = true;
		}
		if (s >= 1.0f)
		{
			saltando = false;
			posicion.y = y_base;
		}
	}
	else
	{
		posicion.y = y_base + 0.6f * sinf(tiempo * 0.8f);
		abre_boca = 0.15f + 0.15f * sinf(tiempo * 1.5f);   // la abre poquito mientras nada
	}

	// Inclinacion del cuerpo: angulo de la velocidad (vertical contra horizontal).
	// En el salto se usa directo al caer al agua regresa poco a poco a horizontal.
	float inclinacion_objetivo = atan2f(vel_vertical, vel_horizontal);
	if (saltando)
		inclinacion = inclinacion_objetivo;
	else
		inclinacion += (inclinacion_objetivo - inclinacion) * glm::min(1.0f, 3.0f * dt);
}

void Reaper::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// Angulos de la animacion
	float coleteo = glm::radians(12.0f) * sinf(tiempo * 2.0f);
	float aleteo = glm::radians(15.0f) * sinf(tiempo * 2.5f);
	float mandibula = glm::radians(30.0f) * abre_boca;
	// Los tentaculos se mueven solos y se abren mucho al atacar
	float tentaculos = glm::radians(8.0f) * sinf(tiempo * 3.0f) + glm::radians(35.0f) * abre_boca;

	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	//Cuerpo
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, -inclinacion, glm::vec3(1.0f, 0.0f, 0.0f));   // nariz arriba/abajo
	model = glm::scale(model, glm::vec3(escala_reaper));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cuerpo.RenderModel();

	//Boca
	model = modelaux;
	model = glm::translate(model, piv_boca);
	model = glm::rotate(model, mandibula, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	boca.RenderModel();

	//Tentaculos de la cara
	model = modelaux;
	model = glm::translate(model, piv_tent_sup_izq);
	model = glm::rotate(model, tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	tent_sup_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_tent_sup_der);
	model = glm::rotate(model, -tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	tent_sup_der.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_tent_inf_izq);
	model = glm::rotate(model, tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	tent_inf_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_tent_inf_der);
	model = glm::rotate(model, -tentaculos, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	tent_inf_der.RenderModel();

	//Aletas
	model = modelaux;
	model = glm::translate(model, piv_aleta_izq);
	model = glm::rotate(model, aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_aleta_der);
	model = glm::rotate(model, -aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_der.RenderModel();

	//Cola
	model = modelaux;
	model = glm::translate(model, piv_cola);
	model = glm::rotate(model, coleteo, glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cola.RenderModel();
}
}
