#pragma once
#include "Escena.h"
#include "Model.h"
#include "PointLight.h"
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cmath>
#include "Terreno.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> ArbolesLuminosos >>>>>>>>>>>>>>>>>>>>>>
// Arboles luminosos en el fondo del lago.
// De noche se prenden solos (frutos brillando + luz puntual morada en cada uno).
// Tecla J: prender / apagar a mano.

class ArbolesLuminosos
{
public:
	static const int num_arboles = 8;

	void Crear(const Terreno& terreno);
	void Actualizar(float dt, bool es_de_noche, Window& ventana);
	void Dibujar(const ContextoRender& ctx);

	// Una luz por arbol (solo cuando estan prendidos)
	PointLight luces[num_arboles];
	glm::vec3 pos_luces[num_arboles];
	unsigned int num_luces = 0;

	bool encendidos = false;

private:
	const Terreno* terreno = nullptr;
	Model arbol;
	float brillo = 0.0f;          // 0 apagado, 1 prendido (cambia poco a poco)
	float tiempo = 0.0f;
	bool era_de_noche = false;
};
}
