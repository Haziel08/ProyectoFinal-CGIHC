#pragma once
#include "Escena.h"
#include "CicloDiaNoche.h"
#include "Terreno.h"
#include "Agua.h"
#include "Vegetacion.h"
#include "Aldea.h"
#include "Peces.h"
#include "Necromorfos.h"
#include "Stalker.h"
#include "Pidgeys.h"
#include "Reaper.h"

class EscenaPlanetaAzul : public Escena
{
public:
	const char* Nombre() const override { return "Planeta Azul"; }
	void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

	// main.cpp usa esta luz cuando el planeta azul esta activo
	DirectionalLight* ObtenerLuzDireccional() override { return &luzSol; }

	// main.cpp usa las luces de la aldea fogata y antorchas
	PointLight* ObtenerLucesPuntuales() override { return aldea.luces; }
	unsigned int NumLucesPuntuales() override { return aldea.numLuces; }

private:
	float tiempo = 0.0f;          // reloj de la escena

	CicloDiaNoche ciclo;
	DirectionalLight luzSol;      // la unica luz direccional del planeta
	Skybox skyboxDia;
	Skybox skyboxNoche;

	Terreno terreno;
	Vegetacion vegetacion;
	Aldea aldea;
	Peces peces;
	Necromorfos necromorfos;
	Stalker stalker;             // avatar
	Pidgeys pidgeys;
	Reaper reaper;
	Agua agua;
	float nivelAgua = -1.0f;      // altura de la superficie del lago

};
