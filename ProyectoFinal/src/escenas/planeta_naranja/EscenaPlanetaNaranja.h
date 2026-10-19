#pragma once
/*
 * EscenaPlanetaNaranja.h — Planeta Naranja
 * Pista de carreras con naves espaciales y el Seamoth.
 *
 * Archivos de esta escena:
 *   código   -> src/escenas/planeta_naranja/   (aquí se pueden agregar más .cpp/.h)
 *   modelos  -> assets/planeta_naranja/modelos/
 *   texturas -> assets/planeta_naranja/texturas/
 *   skybox   -> assets/planeta_naranja/skybox/
 */
#include "Escena.h"

class EscenaPlanetaNaranja : public Escena
{
public:
	const char* Nombre() const override { return "Planeta Naranja"; }
	void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

private:
	// Aquí van los modelos, texturas y variables de animación de la escena
};
