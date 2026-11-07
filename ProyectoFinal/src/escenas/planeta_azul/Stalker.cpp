#include "Stalker.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Stalker >>>>>>>>>>>>>>>>>>>>>>


// Articulaciones
static const glm::vec3 piv_boca_sup     ( 0.00f,  0.05f,  1.08f);
static const glm::vec3 piv_boca_inf     ( 0.00f, -0.02f,  0.98f);
static const glm::vec3 piv_aleta_del_izq ( 0.21f, -0.19f,  0.68f);
static const glm::vec3 piv_aleta_del_der (-0.21f, -0.19f,  0.68f);
static const glm::vec3 piv_aleta_tras_izq( 0.16f, -0.21f, -0.49f);
static const glm::vec3 piv_aleta_tras_der(-0.16f, -0.21f, -0.49f);

// Recorrido elipse dentro del lago
static const float centro_x = 0.0f, centro_z = 70.0f;
static const float radio_x = 45.0f, radio_z = 30.0f;
static const float profundidad = -5.0f;     // el agua esta en y = -1
static const float velocidad = 0.12f;       // radianes por segundo
static const float escala = 1.2f;           

void Stalker::Crear()
{
	cuerpo.LoadModel("assets/planeta_azul/modelos/stalker_cuerpo.obj");
	boca_sup.LoadModel("assets/planeta_azul/modelos/stalker_boca_sup.obj");
	boca_inf.LoadModel("assets/planeta_azul/modelos/stalker_boca_inf.obj");
	aleta_del_izq.LoadModel("assets/planeta_azul/modelos/stalker_aleta_del_izq.obj");
	aleta_del_der.LoadModel("assets/planeta_azul/modelos/stalker_aleta_del_der.obj");
	aleta_tras_izq.LoadModel("assets/planeta_azul/modelos/stalker_aleta_tras_izq.obj");
	aleta_tras_der.LoadModel("assets/planeta_azul/modelos/stalker_aleta_tras_der.obj");
}

void Stalker::Actualizar(float dt)
{
	tiempo += dt;
	angulo += velocidad * dt;

	// Posicion en la elipse, subiendo y bajando un poco
	posicion.x = centro_x + radio_x * cosf(angulo);
	posicion.z = centro_z + radio_z * sinf(angulo);
	posicion.y = profundidad + 0.5f * sinf(tiempo * 1.5f);

	// Mira hacia donde avanza derivada de la elipse = (-A sen t, B cos t)
	giro = atan2f(-radio_x * sinf(angulo), radio_z * cosf(angulo));
}

void Stalker::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// Angulos de la animacion
	float remo = glm::radians(25.0f) * sinf(tiempo * 3.0f);                 // aletas delanteras
	float remo_tras = glm::radians(25.0f) * sinf(tiempo * 3.0f + 3.142f);   // traseras: al reves
	float abre = 0.5f + 0.5f * sinf(tiempo * 1.2f);                         // 0 = cerrada, 1 = abierta
	float vaiven = glm::radians(6.0f) * sinf(tiempo * 3.0f);               // el cuerpo se menea al nadar

	glm::mat4 model(1.0f);
	glm::mat4 modelaux(1.0f);

	//Cuerpo
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro + vaiven, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escala));
	modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cuerpo.RenderModel();

	//Boca superior
	model = modelaux;
	model = glm::translate(model, piv_boca_sup);
	model = glm::rotate(model, glm::radians(-6.0f) * abre, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	boca_sup.RenderModel();

	//Boca inferior
	model = modelaux;
	model = glm::translate(model, piv_boca_inf);
	model = glm::rotate(model, glm::radians(14.0f) * abre, glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	boca_inf.RenderModel();

	//Aletas delanteras
	model = modelaux;
	model = glm::translate(model, piv_aleta_del_izq);
	model = glm::rotate(model, remo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_del_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_aleta_del_der);
	model = glm::rotate(model, -remo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_del_der.RenderModel();

	//Aletas traseras
	model = modelaux;
	model = glm::translate(model, piv_aleta_tras_izq);
	model = glm::rotate(model, remo_tras, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_tras_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_aleta_tras_der);
	model = glm::rotate(model, -remo_tras, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_tras_der.RenderModel();
}
}
