#pragma once
// Piso: un cubo aplastado con textura repetida
#include "Escena.h"
#include "Mesh_tn.h"
#include "Texture.h"
#include "Material.h"

class Piso
{
public:
	float tamano = 600.0f;            // lado del piso
	float grosor = 2.0f;              // alto del cubo (la cara de arriba queda en y = 0)
	float metrosPorTextura = 12.0f;   // cada cuanto se repite la textura

	void Crear(const char* rutaTextura);
	void Dibujar(const ContextoRender& ctx);
	~Piso();

private:
	MeshModel* cubo = nullptr;
	Texture textura;
	Material material = Material(0.3f, 16);
};
