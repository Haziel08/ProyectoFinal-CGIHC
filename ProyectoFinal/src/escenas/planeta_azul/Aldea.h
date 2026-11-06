#pragma once
// Aldea
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
	void DibujarLlamas(const ContextoRender& ctx, float tiempo);   // fuego

	// Luces puntuales encendidas en este momento
	PointLight luces[max_point_lights];
	unsigned int numLuces = 0;

	// Donde esta cada choza, para poner a sus habitantes
	int NumChozas() const;
	glm::vec2 PosicionChoza(int i) const;

	bool fogataEncendida = false;      // empieza apagada; se prende con F
	bool antorchasEncendidas = false;   // las prende el ciclo de dia y noche

private:
	glm::vec3 SobreElSuelo(float x, float z) const;   //x, altura del terreno, z

	const Terreno* terreno = nullptr;
	Fuego fuego;

	Model ChozaMuu_M;
	Model ChozaMantah_M;
	Model ChozaKrakee_M;
	Model ChozaAntuk_M;
	Model Antorcha_M;
	Model Fogata_M;
};
