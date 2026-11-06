#include "Stalker.h"

#include <cmath>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Articulaciones: donde se pega cada pieza al cuerpo (en metros)
static const glm::vec3 pivBocaSup     ( 0.00f,  0.05f,  1.08f);
static const glm::vec3 pivBocaInf     ( 0.00f, -0.02f,  0.98f);
static const glm::vec3 pivAletaDelIzq ( 0.21f, -0.19f,  0.68f);
static const glm::vec3 pivAletaDelDer (-0.21f, -0.19f,  0.68f);
static const glm::vec3 pivAletaTrasIzq( 0.16f, -0.21f, -0.49f);
static const glm::vec3 pivAletaTrasDer(-0.16f, -0.21f, -0.49f);

// Recorrido: elipse dentro del lago
static const float centroX = 0.0f, centroZ = 70.0f;
static const float radioX = 45.0f, radioZ = 30.0f;
static const float profundidad = -5.0f;     // el agua esta en y = -1
static const float velocidad = 0.12f;       // radianes por segundo
static const float escala = 1.2f;           // el modelo mide ~5 m, asi queda de ~6 m

void Stalker::Crear()
{
	Cuerpo_M.LoadModel("assets/planeta_azul/modelos/Stalker_Cuerpo.obj");
	BocaSup_M.LoadModel("assets/planeta_azul/modelos/Stalker_BocaSup.obj");
	BocaInf_M.LoadModel("assets/planeta_azul/modelos/Stalker_BocaInf.obj");
	AletaDelIzq_M.LoadModel("assets/planeta_azul/modelos/Stalker_AletaDelIzq.obj");
	AletaDelDer_M.LoadModel("assets/planeta_azul/modelos/Stalker_AletaDelDer.obj");
	AletaTrasIzq_M.LoadModel("assets/planeta_azul/modelos/Stalker_AletaTrasIzq.obj");
	AletaTrasDer_M.LoadModel("assets/planeta_azul/modelos/Stalker_AletaTrasDer.obj");
}

void Stalker::Actualizar(float dt)
{
	tiempo += dt;
	angulo += velocidad * dt;

	// Posicion en la elipse, subiendo y bajando un poco
	posicion.x = centroX + radioX * cosf(angulo);
	posicion.z = centroZ + radioZ * sinf(angulo);
	posicion.y = profundidad + 0.5f * sinf(tiempo * 1.5f);

	// Mira hacia donde avanza: derivada de la elipse = (-A sen t, B cos t)
	giro = atan2f(-radioX * sinf(angulo), radioZ * cosf(angulo));
}

void Stalker::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// Angulos de la animacion
	float remo = glm::radians(25.0f) * sinf(tiempo * 3.0f);                 // aletas delanteras
	float remoTras = glm::radians(25.0f) * sinf(tiempo * 3.0f + 3.1416f);   // traseras: al reves
	float abre = 0.5f + 0.5f * sinf(tiempo * 1.2f);                         // 0 = cerrada, 1 = abierta
	float vaiven = glm::radians(6.0f) * sinf(tiempo * 3.0f);               // el cuerpo se menea al nadar

	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	// --- Cuerpo (padre de todo) ---
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro + vaiven, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escala));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	Cuerpo_M.RenderModel();

	// --- Boca superior: sube un poco al abrir ---
	model = modelaux;
	model = glm::translate(model, pivBocaSup);
	model = glm::rotate(model, glm::radians(-6.0f) * abre, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	BocaSup_M.RenderModel();

	// --- Boca inferior: baja al abrir ---
	model = modelaux;
	model = glm::translate(model, pivBocaInf);
	model = glm::rotate(model, glm::radians(14.0f) * abre, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	BocaInf_M.RenderModel();

	// --- Aletas delanteras (izq gira +, der gira - para que ambas suban juntas) ---
	model = modelaux;
	model = glm::translate(model, pivAletaDelIzq);
	model = glm::rotate(model, remo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	AletaDelIzq_M.RenderModel();

	model = modelaux;
	model = glm::translate(model, pivAletaDelDer);
	model = glm::rotate(model, -remo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	AletaDelDer_M.RenderModel();

	// --- Aletas traseras ---
	model = modelaux;
	model = glm::translate(model, pivAletaTrasIzq);
	model = glm::rotate(model, remoTras, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	AletaTrasIzq_M.RenderModel();

	model = modelaux;
	model = glm::translate(model, pivAletaTrasDer);
	model = glm::rotate(model, -remoTras, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	AletaTrasDer_M.RenderModel();
}
