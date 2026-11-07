#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de EscenaPlanetaMorado >>>>>>>>>>>>>>>>>>>>>>
/*
 * EscenaPlanetaMorado.h — Planeta Morado
 * Plataforma de lanzamiento Neptuno y cohete, módulos de Subnautica, corales y arbusto bulbo.
 *
 * Archivos de esta escena:
 *   código   -> src/escenas/planeta_morado/
 *   modelos  -> assets/planeta_morado/modelos/
 *   texturas -> assets/planeta_morado/texturas/
 *   skybox   -> assets/planeta_morado/texturas/skybox/
 */
#include "Escena.h"
#include "../planeta_azul/CicloDiaNoche.h"
#include "Piso.h"

class EscenaPlanetaMorado : public Escena
{
public:
	const char* Nombre() const override { return "Planeta Morado"; }
	void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

	// main.cpp usa esta luz cuando el planeta morado esta activo
	DirectionalLight* ObtenerLuzDireccional() override { return &luzSol; }

private:
	float tiempo = 0.0f;

	CicloDiaNoche ciclo;          // el mismo ciclo del planeta azul, con otros colores
	DirectionalLight luzSol;
	Skybox skyboxDia;
	Skybox skyboxNoche;

	Piso piso;
};
//>>>>>>>>>>>>>>>>>>>>>> EscenaPlanetaMorado >>>>>>>>>>>>>>>>>>>>>>
