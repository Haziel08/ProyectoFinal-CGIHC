#pragma once
// Aldea: chozas tribales alrededor de una fogata, con antorchas.
//   - La fogata se prende y apaga con la tecla F.
//   - Las antorchas se prenden solas de noche (ciclo de dia y noche).
//   - Las luces puntuales van en un arreglo, como en las practicas
//     (el shader acepta hasta max_point_lights = 16).
// Modelos: Wooden Torch y Bonfire de Quaternius (CC0), Stylized Tribal Hut.
#include "Escena.h"
#include "Model.h"
#include "Terreno.h"
#include "Fuego.h"

class Aldea
{
public:
	glm::vec2 centro = glm::vec2(-110.0f, -40.0f);   // centro de la aldea = la fogata

	void Crear(const Terreno& terreno);
	void Actualizar(float tiempo, bool esDeNoche, Window& ventana);
	void Dibujar(const ContextoRender& ctx);                 // chozas, antorchas y fogata
	void DibujarLlamas(const ContextoRender& ctx, float tiempo);   // fuego (al final, es transparente)

	// Luces puntuales encendidas en este momento (las usa main.cpp)
	PointLight luces[max_point_lights];
	unsigned int numLuces = 0;

	bool fogataEncendida = true;
	bool antorchasEncendidas = false;   // las prende el ciclo de dia y noche

private:
	glm::vec3 SobreElSuelo(float x, float z) const;   // (x, altura del terreno, z)

	const Terreno* terreno = nullptr;
	Fuego fuego;

	Model ChozaMuu_M;
	Model ChozaMantah_M;
	Model ChozaKrakee_M;
	Model ChozaAntuk_M;
	Model Antorcha_M;
	Model Fogata_M;
};
