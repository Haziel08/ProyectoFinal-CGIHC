#pragma once
/*
 * EscenaPlanetaAzul.h — Planeta Azul
 * Criaturas (Stalker, Air fish, Reaper, Mitón, necromorfo) y chozas tribales.
 *
 * Partes de la escena (cada una en su archivo):
 *   CicloDiaNoche  -> luz direccional (sol/luna) y cambio de skybox
 *   Terreno        -> pasto, colinas y el lago con fondo de arena
 *   Agua           -> superficie del lago (shader_animado)
 *   Vegetacion     -> arboles, pasto, flores y rocas (deja zonas libres)
 *   Aldea          -> chozas, antorchas y fogata (tecla F) con luces puntuales
 *   Fuego          -> llamas animadas (shader_animado, el mismo del agua)
 *   Peces          -> Air fish y Peepers nadando en el lago
 *   Necromorfos    -> NPCs con animacion esqueletal (caminan; bailan con la fogata)
 *   Stalker        -> AVATAR jerarquico que nada en el lago (aletas y boca)
 *   Pidgeys        -> aves jerarquicas volando en el cielo (aletean)
 *   Reaper         -> leviatan jerarquico; tecla G: salta y se come un Pidgey
 *
 * Archivos:
 *   código   -> src/escenas/planeta_azul/
 *   modelos  -> assets/planeta_azul/modelos/
 *   texturas -> assets/planeta_azul/texturas/
 *   skybox   -> assets/planeta_azul/texturas/skybox/dia/  y  .../skybox/noche/
 */
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

	// main.cpp usa las luces de la aldea (fogata y antorchas)
	PointLight* ObtenerLucesPuntuales() override { return aldea.luces; }
	unsigned int NumLucesPuntuales() override { return aldea.numLuces; }

private:
	float tiempo = 0.0f;          // reloj de la escena (para animaciones)

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

	// Aquí van los modelos (chozas, criaturas...). Usar terreno.Altura(x, z)
	// para ponerlos sobre el suelo, o una y menor a nivelAgua para meterlos al lago.
	// Zonas despejadas para ellos (ver Vegetacion::EnZonaLibre):
	//   aldea (ya tiene chozas, antorchas y fogata)  centro (-110, -40)  radio 50
	//   criaturas / NPCs   centro ( 110, -30)  radio 45
	//   entrada            centro (   0, -65)  radio 30
};
