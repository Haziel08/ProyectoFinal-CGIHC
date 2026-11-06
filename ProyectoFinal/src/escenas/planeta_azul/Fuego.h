#pragma once
// Fuego
#include <glm.hpp>
#include "Escena.h"
#include "Mesh_tn.h"
#include "Texture.h"
#include "ShaderAnimado.h"

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
