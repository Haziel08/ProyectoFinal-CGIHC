#pragma once
// Stalker: el AVATAR del planeta azul. Modelo jerarquico que nada en el lago.
//
//   Jerarquia:
//       Cuerpo
//        |-- Boca superior        (gira en X: abre hacia arriba)
//        |-- Boca inferior        (gira en X: abre hacia abajo)
//        |-- Aleta delantera izq  (gira en Z: rema arriba/abajo)
//        |-- Aleta delantera der
//        |-- Aleta trasera izq    (igual, pero desfasada media vuelta)
//        |-- Aleta trasera der
//
//   Recorrido: elipse dentro del lago   x = cx + A cos(t),  z = cz + B sen(t)
//   Las aletas reman con un seno y la boca se abre y cierra con otro.
//   Su material es mas brillante (Ks y Ns en los .mtl) para que refleje las luces.
#include "Escena.h"
#include "Model.h"

class Stalker
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

	glm::vec3 posicion = glm::vec3(0.0f);   // donde esta ahora (por si una camara lo quiere seguir)
	float giro = 0.0f;                       // hacia donde mira (radianes)

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
