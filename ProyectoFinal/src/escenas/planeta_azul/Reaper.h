#pragma once
#include "Escena.h"
#include "Model.h"
#include "Material.h"
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cmath>
#include <string>
#include <vector>
#include "Pidgeys.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Reaper >>>>>>>>>>>>>>>>>>>>>>
// Reaper

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
	bool ya_mordio = false;
	float abre_boca = 0.0f;                   // 0 cerrada, 1 abierta

	Model cuerpo;
	Model boca;
	Model tent_sup_izq;
	Model tent_sup_der;
	Model tent_inf_izq;
	Model tent_inf_der;
	Model aleta_izq;
	Model aleta_der;
	Model cola;
};
}
