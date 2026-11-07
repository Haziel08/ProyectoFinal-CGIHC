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
#include "Terreno.h"
#include "Fuego.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Aldea >>>>>>>>>>>>>>>>>>>>>>
// Aldea

class Aldea
{
public:
	glm::vec2 centro = glm::vec2(-110.0f, -40.0f);   // centro de la aldea = la fogata

	void Crear(const Terreno& terreno);
	void Actualizar(float tiempo, bool es_de_noche, Window& ventana);
	void Dibujar(const ContextoRender& ctx);                 // chozas, antorchas y fogata
	void DibujarLlamas(const ContextoRender& ctx, float tiempo);   // fuego

	// Luces puntuales encendidas en este momento
	PointLight luces[max_point_lights];
	unsigned int num_luces = 0;

	// Donde esta cada choza, para poner a sus habitantes
	int NumChozas() const;
	glm::vec2 PosicionChoza(int i) const;

	bool fogata_encendida = false;      // empieza apagada; se prende con F
	bool antorchas_encendidas = false;   // las prende el ciclo de dia y noche

private:
	glm::vec3 SobreElSuelo(float x, float z) const;   //x, altura del terreno, z

	const Terreno* terreno = nullptr;
	Fuego fuego;

	Model choza_muu;
	Model choza_mantah;
	Model choza_krakee;
	Model choza_antuk;
	Model antorcha;
	Model fogata;
};
}
