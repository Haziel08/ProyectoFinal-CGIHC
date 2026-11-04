#pragma once
// Fuego: llama animada hecha con dos cuadros cruzados (como el agave de las
// practicas) y el mismo shader del agua (shader_animado):
//   - la textura sube con el tiempo (velocidadUV)
//   - los vertices se mueven con ondas seno (la llama "baila")
//   - se mezcla sumando color (lo negro de la textura no se ve)
// Un solo objeto Fuego sirve para todas las llamas: se dibuja en varias posiciones.
#include <glm.hpp>
#include "Escena.h"
#include "Mesh_tn.h"
#include "Texture.h"
#include "ShaderAnimado.h"

class Fuego
{
public:
	void Crear();
	// posicion = base de la llama, ancho y alto en metros
	void Dibujar(const ContextoRender& ctx, glm::vec3 posicion, float ancho, float alto, float tiempo);
	~Fuego();

private:
	MeshModel* malla = nullptr;
	Texture textura;
	ShaderAnimado shader;
};
