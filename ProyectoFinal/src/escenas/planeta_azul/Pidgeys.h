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
//>>>>>>>>>>>>>>>>>>>>>> Pidgeys >>>>>>>>>>>>>>>>>>>>>>
// Pidgeys

class Pidgeys
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

	//La presa (la usa el Reaper para saber donde saltar)
	glm::vec2 centro_presa = glm::vec2(0.0f, 70.0f);   // circulo sobre el lago
	float radio_presa = 22.0f;
	float altura_presa = 12.0f;
	float velocidad_presa = 0.25f;                     // radianes por segundo

	float AnguloPresa() const { return angulo_presa; }
	glm::vec3 PosicionPresa() const;
	bool PresaViva() const { return presa_viva; }
	void ComerPresa();                                // la llama el Reaper al morder

private:
	void DibujarAve(const ContextoRender& ctx, glm::vec3 pos, float giro, float alabeo, float aleteo, float escala);

	float tiempo = 0.0f;
	float angulo_presa = 0.0f;
	bool presa_viva = true;
	float tiempo_comida = 0.0f;     // segundos desde que se la comieron

	Model cuerpo;
	Model ala_izq;
	Model ala_der;
};
}
