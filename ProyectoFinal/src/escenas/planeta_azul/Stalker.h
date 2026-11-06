#pragma once
// Stalker
#include "Escena.h"
#include "Model.h"

class Stalker
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

	glm::vec3 posicion = glm::vec3(0.0f);   // donde esta ahora
	float giro = 0.0f;                       // hacia donde mira

private:
	float tiempo = 0.0f;
	float angulo = 0.0f;    // donde va en la elipse

	Model Cuerpo_M;
	Model BocaSup_M;
	Model BocaInf_M;
	Model AletaDelIzq_M;
	Model AletaDelDer_M;
	Model AletaTrasIzq_M;
	Model AletaTrasDer_M;
};
