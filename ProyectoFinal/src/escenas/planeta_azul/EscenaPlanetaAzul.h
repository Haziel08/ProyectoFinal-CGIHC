#pragma once
/*
 * EscenaPlanetaAzul.h — Planeta Azul
 * Criaturas (Stalker, Air fish, Reaper, Mitón, necromorfo) y chozas tribales.
 *
 * Partes de la escena (cada una en su archivo):
 *   CicloDiaNoche  -> luz direccional (sol/luna) y cambio de skybox
 *   Terreno        -> pasto, colinas y el lago con fondo de arena
 *   Agua           -> superficie del lago (shader_animado)
 *
 * Archivos:
 *   código   -> src/escenas/planeta_azul/
 *   modelos  -> assets/planeta_azul/modelos/
 *   texturas -> assets/planeta_azul/texturas/
 *   skybox   -> assets/planeta_azul/skybox/dia/  y  skybox/noche/
 */
#include "Escena.h"
#include "CicloDiaNoche.h"
#include "Terreno.h"
#include "Agua.h"

class EscenaPlanetaAzul : public Escena
{
public:
	const char* Nombre() const override { return "Planeta Azul"; }
	void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

	// main.cpp usa esta luz cuando el planeta azul esta activo
	DirectionalLight* ObtenerLuzDireccional() override { return &luzSol; }

private:
	float tiempo = 0.0f;          // reloj de la escena (para animaciones)

	CicloDiaNoche ciclo;
	DirectionalLight luzSol;      // la unica luz direccional del planeta
	Skybox skyboxDia;
	Skybox skyboxNoche;

	Terreno terreno;
	Agua agua;
	float nivelAgua = -1.0f;      // altura de la superficie del lago

	// Aquí van los modelos (chozas, criaturas...). Usar terreno.Altura(x, z)
	// para ponerlos sobre el suelo, o una y menor a nivelAgua para meterlos al lago.
};
