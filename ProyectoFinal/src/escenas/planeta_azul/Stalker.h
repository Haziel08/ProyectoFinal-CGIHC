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

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Stalker >>>>>>>>>>>>>>>>>>>>>>
// Stalker

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

	Model cuerpo;
	Model boca_sup;
	Model boca_inf;
	Model aleta_del_izq;
	Model aleta_del_der;
	Model aleta_tras_izq;
	Model aleta_tras_der;
};
}
