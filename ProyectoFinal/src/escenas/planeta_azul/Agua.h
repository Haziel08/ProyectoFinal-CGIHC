#pragma once
// Agua superficie semitransparente con olas, dibujada con ShaderAnimado.
// IMPORTANTE: dibujarla AL FINAL de la escena
#include <glm.hpp>
#include "Escena.h"
#include "Mesh_tn.h"
#include "Texture.h"
#include "ShaderAnimado.h"
#include "CicloDiaNoche.h"

class Agua
{
public:
	//Aspecto
	float amplitud = 0.25f;                         // altura de las olas
	float frecuencia = 0.15f;                       // olas mas juntas o separadas
	float velocidad = 1.2f;                         // rapidez de las olas
	glm::vec2 velocidadUV = glm::vec2(0.02f, 0.01f); // como fluye la textura
	float transparencia = 0.6f;                     // 0 invisible, 1 opaca
	glm::vec3 colorBase = glm::vec3(0.55f, 0.8f, 1.0f);

	// Rectangulo de agua (centro, tamano) a la altura nivelY
	void Crear(glm::vec2 centro, glm::vec2 tamano, float nivelY, const char* rutaTextura);
	void Dibujar(const ContextoRender& ctx, float tiempo, const DatosLuz& luz);
	~Agua();

private:
	MeshModel* malla = nullptr;
	Texture textura;
	ShaderAnimado shader;
	float repeticion = 1.0f;
};
