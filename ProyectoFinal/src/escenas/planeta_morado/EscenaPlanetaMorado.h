#pragma once
/*
 * EscenaPlanetaMorado.h — Planeta Morado
 * Plataforma de lanzamiento Neptuno y cohete, módulos de Subnautica, corales y arbusto bulbo.
 *
 * Archivos de esta escena:
 *   código   -> src/escenas/planeta_morado/   (aquí se pueden agregar más .cpp/.h)
 *   modelos  -> assets/planeta_morado/modelos/
 *   texturas -> assets/planeta_morado/texturas/
 *   skybox   -> assets/planeta_morado/skybox/
 */
#include "Escena.h"

class EscenaPlanetaMorado : public Escena
{
public:
	const char* Nombre() const override { return "Planeta Morado"; }
	void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

private:
	// Aquí van los modelos, texturas y variables de animación de la escena
};
