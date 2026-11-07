#pragma once
#include "Escena.h"
#include "Model.h"
#include "Material.h"
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cmath>
#include <string>
#include <vector>
#include "Terreno.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Vegetacion >>>>>>>>>>>>>>>>>>>>>>
// Vegetacion: arboles, arbustos, pasto, flores y rocas del planeta azul.
// Modelos: Stylized Nature MegaKit de Quaternius (licencia CC0).
//
// Igual que en las practicas: cada modelo se carga una vez (Model) y se dibuja
// varias veces cambiando su matriz model (trasladar, rotar, escalar).

class Vegetacion
{
public:
	void Crear(const Terreno& terreno);
	void Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int num_luces);

	// Zonas sin vegetacion para casas y NPCs (true si (x, z) cae dentro de una)
	bool EnZonaLibre(float x, float z) const;

private:
	// Dibuja un modelo parado sobre el terreno en (x, z)
	void DibujarEn(Model& modelo, float x, float z, float escala, float giro, glm::vec3 color = glm::vec3(1.0f));

	const Terreno* terreno = nullptr;
	Shader shader;                          // shader_vegetacion (recorta lo transparente de las hojas)
	Material material_plantas = Material(0.3f, 8);
	GLuint u_model = 0, u_color = 0;

	Model arbol1;
	Model arbol2;
	Model pino1;
	Model pino2;
	Model arbusto;
	Model arbusto_flores;
	Model pasto;
	Model flores;
	Model roca1;
	Model roca2;
	Model piedra;
	Model alga;
};
}
