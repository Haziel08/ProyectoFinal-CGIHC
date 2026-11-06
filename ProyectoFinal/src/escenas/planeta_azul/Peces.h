#pragma once
// Peces: Air fish y Peepers nadando dentro del lago (animacion compleja continua).
//
//   Air fish -> nadan en forma de "8" (curva de Lissajous):
//                 x = cx + A * sen(t)        z = cz + B * sen(2t) / 2
//   Peepers  -> nadan en circulo, en grupo (cardumen):
//                 x = cx + R * cos(t)        z = cz + R * sen(t)
//
//   - El pez gira para mirar hacia donde avanza: giro = atan2(dx, dz),
//     donde (dx, dz) es la derivada (la direccion del movimiento).
//   - Sube y baja un poco con un seno, y mueve el cuerpo de lado a lado
//     (coleteo) con otro seno mas rapido.
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
