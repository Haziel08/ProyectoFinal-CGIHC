#pragma once
/*
 * EscenaPlanetaAzul.h — Planeta Azul
 * Criaturas (Stalker, Air fish, Reaper, Mitón, necromorfo) y chozas tribales.
 *
 * Archivos de esta escena:
 *   código   -> src/escenas/planeta_azul/   (aquí se pueden agregar más .cpp/.h)
 *   modelos  -> assets/planeta_azul/modelos/
 *   texturas -> assets/planeta_azul/texturas/
 *   skybox   -> assets/planeta_azul/skybox/
 */
#include "Escena.h"

class EscenaPlanetaAzul : public Escena
{
public:
	const char* Nombre() const override { return "Planeta Azul"; }
	void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

private:
	// Aquí van los modelos, texturas y variables de animación de la escena
};
