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
//>>>>>>>>>>>>>>>>>>>>>> Terreno >>>>>>>>>>>>>>>>>>>>>>
// Terreno

class Terreno
{
public:
	//Medidas
	float tamano = 400.0f;                        // lado del terreno cuadrado
	int celdas = 400;                             // cuadritos por lado
	glm::vec2 centro_lago = glm::vec2(0.0f, 70.0f);
	glm::vec2 radio_lago = glm::vec2(80.0f, 55.0f); // radio en X y en Z
	float profundidad = 30.0f;                    // que tan hondo es el lago
	float metros_por_textura = 12.0f;               // cada cuanto se repite la textura

	void Crear();
	void Dibujar(const ContextoRender& ctx);
	~Terreno();

	float Altura(float x, float z) const;         // altura del suelo en (x, z)
	bool EsLago(float x, float z) const;          // true si (x, z) cae en el lago o la playa
	float DistanciaLago(float x, float z) const;  // 0 en el centro, 1 en la orilla, >1 tierra

private:
	glm::vec3 NormalEn(float x, float z) const;
	void AgregarCuadro(std::vector<GLfloat>& v, std::vector<unsigned int>& idx, float x0, float z0, float lado);

	MeshModel* tierra = nullptr;
	MeshModel* fondo = nullptr;
	Texture textura_pasto;
	Texture textura_arena;
	Material material_suelo = Material(0.2f, 8);
};
}
