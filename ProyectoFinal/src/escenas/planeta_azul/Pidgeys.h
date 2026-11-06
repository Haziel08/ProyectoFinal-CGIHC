#pragma once
// Pidgeys
#include "Escena.h"
#include "Model.h"

class Pidgeys
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

	//La presa (la usa el Reaper para saber donde saltar)
	glm::vec2 centroPresa = glm::vec2(0.0f, 70.0f);   // circulo sobre el lago
	float radioPresa = 22.0f;
	float alturaPresa = 12.0f;
	float velocidadPresa = 0.25f;                     // radianes por segundo

	float AnguloPresa() const { return anguloPresa; }
	glm::vec3 PosicionPresa() const;
	bool PresaViva() const { return presaViva; }
	void ComerPresa();                                // la llama el Reaper al morder

private:
	void DibujarAve(const ContextoRender& ctx, glm::vec3 pos, float giro, float alabeo, float aleteo, float escala);

	float tiempo = 0.0f;
	float anguloPresa = 0.0f;
	bool presaViva = true;
	float tiempoComida = 0.0f;     // segundos desde que se la comieron

	Model Cuerpo_M;
	Model AlaIzq_M;
	Model AlaDer_M;
};
