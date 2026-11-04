#pragma once
// Terreno: piso del planeta azul hecho con codigo (una malla de cuadritos).
//   - Zona de tierra con pasto y colinas suaves.
//   - Un lago ovalado y profundo; su fondo usa textura de arena (piso marino).
// Altura(x, z) sirve para poner casas o criaturas justo sobre el suelo.
#include <glm.hpp>
#include "Escena.h"
#include "Mesh_tn.h"
#include "Texture.h"
#include "Material.h"

class Terreno
{
public:
	// --- Medidas (se pueden ajustar antes de Crear) ---
	float tamano = 400.0f;                        // lado del terreno cuadrado
	int celdas = 200;                             // cuadritos por lado (mas = mas detalle)
	glm::vec2 centroLago = glm::vec2(0.0f, 70.0f);
	glm::vec2 radioLago = glm::vec2(80.0f, 55.0f); // radio en X y en Z
	float profundidad = 30.0f;                    // que tan hondo es el lago
	float metrosPorTextura = 12.0f;               // cada cuanto se repite la textura

	void Crear();
	void Dibujar(const ContextoRender& ctx);
	~Terreno();

	float Altura(float x, float z) const;         // altura del suelo en (x, z)
	bool EsLago(float x, float z) const;          // true si (x, z) cae en el lago o la playa
	float DistanciaLago(float x, float z) const;  // 0 en el centro, 1 en la orilla, >1 tierra

private:
	glm::vec3 NormalEn(float x, float z) const;
	void CrearPlaya();                            // tira de arena que sigue el ovalo del lago

	MeshModel* tierra = nullptr;
	MeshModel* fondo = nullptr;
	MeshModel* playa = nullptr;
	Texture texturaPasto;
	Texture texturaArena;
	Material materialSuelo = Material(0.2f, 8);
};
