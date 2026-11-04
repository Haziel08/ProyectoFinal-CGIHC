#pragma once
// Vegetacion: arboles, arbustos, pasto, flores y rocas del planeta azul.
// Modelos: Stylized Nature MegaKit de Quaternius (licencia CC0).
//
// Igual que en las practicas: cada modelo se carga una vez (Model) y se dibuja
// varias veces cambiando su matriz model (trasladar, rotar, escalar).
// Las posiciones estan escritas a mano en Vegetacion.cpp.
#include "Escena.h"
#include "Model.h"
#include "Material.h"
#include "Terreno.h"

class Vegetacion
{
public:
	void Crear(const Terreno& terreno);
	void Dibujar(const ContextoRender& ctx, DirectionalLight* luz);

	// Zonas sin vegetacion para casas y NPCs (true si (x, z) cae dentro de una)
	bool EnZonaLibre(float x, float z) const;

private:
	// Dibuja un modelo parado sobre el terreno en (x, z)
	void DibujarEn(Model& modelo, float x, float z, float escala, float giro, glm::vec3 color = glm::vec3(1.0f));

	const Terreno* terreno = nullptr;
	Shader shader;                          // shader_vegetacion (recorta lo transparente de las hojas)
	Material materialPlantas = Material(0.3f, 8);
	GLuint uModel = 0, uColor = 0;

	Model Arbol1_M;
	Model Arbol2_M;
	Model Pino1_M;
	Model Pino2_M;
	Model Arbusto_M;
	Model ArbustoFlores_M;
	Model Pasto_M;
	Model Flores_M;
	Model Roca1_M;
	Model Roca2_M;
	Model Piedra_M;
	Model Alga_M;
};
