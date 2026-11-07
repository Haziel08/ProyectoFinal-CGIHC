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
#include "ShaderAnimado.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Fuego >>>>>>>>>>>>>>>>>>>>>>
// Fuego

class Fuego
{
public:
	void Crear();
	// posicion = base de la llama, ancho y alto
	void Dibujar(const ContextoRender& ctx, glm::vec3 posicion, float ancho, float alto, float tiempo);
	~Fuego();

private:
	MeshModel* malla = nullptr;
	Texture textura;
	ShaderAnimado shader;
};
}
