#pragma once
/*
 * EscenaEspacio.h — Espacio
 * Estación espacial XQ6 Platform, Nave Aurora, satélites, lunas y asteroides.
 *
 * Archivos de esta escena:
 *   código   -> src/escenas/espacio/   (aquí se pueden agregar más .cpp/.h)
 *   modelos  -> assets/espacio/modelos/
 *   texturas -> assets/espacio/texturas/
 *   skybox   -> assets/espacio/skybox/
 */
#include "Escena.h"

#include "Mesh_tn.h"
#include "Texture.h"
#include "Material.h"

class EscenaEspacio : public Escena
{
public:
	const char* Nombre() const override { return "Espacio"; }
	void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

private:
	// Aquí van los modelos, texturas y variables de animación de la escena
	MeshModel* esferaPrueba = nullptr;
	Texture texturaBlanca;
	Material materialOpaco = Material(0.3f, 4);
};
