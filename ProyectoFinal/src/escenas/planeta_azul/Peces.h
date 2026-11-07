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
//>>>>>>>>>>>>>>>>>>>>>> Peces >>>>>>>>>>>>>>>>>>>>>>
// Peces Air fish y Peepers

class Peces
{
public:
	void Crear();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx);

private:
	float tiempo = 0.0f;
	Model air_fish;
	Model peeper;
};
}
