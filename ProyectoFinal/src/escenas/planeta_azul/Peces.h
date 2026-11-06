#pragma once
// Peces Air fish y Peepers
#include "Escena.h"
#include "Model.h"

class Peces
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

private:
	float tiempo = 0.0f;
	Model AirFish_M;
	Model Peeper_M;
};
