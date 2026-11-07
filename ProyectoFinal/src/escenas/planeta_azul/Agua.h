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
#include "CicloDiaNoche.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Agua >>>>>>>>>>>>>>>>>>>>>>
// Agua superficie semitransparente con olas, dibujada con ShaderAnimado.
// IMPORTANTE: dibujarla AL FINAL de la escena

class Agua
{
public:
	//Aspecto
	float amplitud = 0.25f;                         // altura de las olas
	float frecuencia = 0.15f;                       // olas mas juntas o separadas
	float velocidad = 1.2f;                         // rapidez de las olas
	glm::vec2 velocidad_u_v = glm::vec2(0.02f, 0.01f); // como fluye la textura
	float transparencia = 0.6f;                     // 0 invisible, 1 opaca
	glm::vec3 color_base = glm::vec3(0.55f, 0.8f, 1.0f);

	// Rectangulo de agua (centro, tamano) a la altura nivel_y
	void Crear(glm::vec2 centro, glm::vec2 tamano, float nivel_y, const char* ruta_textura);
	void Dibujar(const ContextoRender& ctx, float tiempo, const DatosLuz& luz);
	~Agua();

private:
	MeshModel* malla = nullptr;
	Texture textura;
	ShaderAnimado shader;
	float repeticion = 1.0f;
};
}
