#pragma once
// Reaper
#include "Escena.h"
#include "Model.h"
#include "Pidgeys.h"

class Reaper
{
public:
	void Crear();
	void Actualizar(float dt, Window& ventana, Pidgeys& pidgeys);
	void Dibujar(const ContextoRender& ctx);

private:
	float tiempo = 0.0f;

	glm::vec3 posicion = glm::vec3(0.0f);   // posicion de la cabeza
	float giro = 0.0f;                       // hacia donde mira
	float inclinacion = 0.0f;                // nariz arriba (+) o abajo (-)

	bool saltando = false;
	float s = 0.0f;                          // avance del salto (0 a 1)
	bool yaMordio = false;
	float abreBoca = 0.0f;                   // 0 cerrada, 1 abierta

	Model Cuerpo_M;
	Model Boca_M;
	Model TentSupIzq_M;
	Model TentSupDer_M;
	Model TentInfIzq_M;
	Model TentInfDer_M;
	Model AletaIzq_M;
	Model AletaDer_M;
	Model Cola_M;
};
