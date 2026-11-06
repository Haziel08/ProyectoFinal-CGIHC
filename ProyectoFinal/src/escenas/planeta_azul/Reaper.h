#pragma once
// Reaper Leviathan: depredador del lago (modelo jerarquico).
//
//   Jerarquia (el cuerpo gira desde la CABEZA, asi la boca llega a la presa):
//       Cuerpo
//        |-- Boca (mandibula)          gira en X
//        |-- Tentaculo superior izq    gira en Y (se abren hacia afuera)
//        |-- Tentaculo superior der
//        |-- Tentaculo inferior izq
//        |-- Tentaculo inferior der
//        |-- Aleta izq / Aleta der     giran en Z
//        |-- Cola                      gira en Y (coletea)
//
//   Nada en circulo bajo el agua, siempre debajo del Pidgey "presa".
//   Tecla G: salta. La altura es una parabola:
//        y = yBase + H * 4 s (1 - s)      con s de 0 a 1
//   En s = 0.5 llega arriba (a la altura de la presa), muerde y se la come,
//   y cae de nuevo al agua. El cuerpo se inclina segun la velocidad vertical.
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
	float giro = 0.0f;                       // hacia donde mira (eje Y)
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
