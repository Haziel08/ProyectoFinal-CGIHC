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
#include "ModeloAnimado.h"
#include "Terreno.h"
#include "Aldea.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Necromorfos >>>>>>>>>>>>>>>>>>>>>>
// Necromorfos NPCs con animacion esqueletal

class Necromorfos
{
public:
    glm::vec3 PosicionPatrulla() const;
    glm::vec3 RumboPatrulla() const;
	void Crear(const Terreno& terreno, const Aldea& aldea);
	void Actualizar(float dt, bool fogata_encendida);
	void Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int num_luces);

private:
	// Un necromorfo que da vueltas a un circulo
	struct Caminante
	{
		glm::vec2 centro;
		float radio;
		float angulo;        // donde va en el circulo (radianes)
		float velocidad;     // radianes por segundo (negativa = al reves)
		bool baila_con_fogata;
	};

	void DibujarUno(const Caminante& c, int animacion, float tiempo_animacion, GLuint u_model);

	const Terreno* terreno = nullptr;
	glm::vec2 fogata;
	bool bailando = false;
	float tiempo = 0.0f;

	ModeloAnimado necromorfo;
	int anim_caminar = 0;
	int anim_bailar = 1;
	Shader shader;              // shader_esqueleto.vert + shader_light.frag

	static const int max_caminantes = 10;
	Caminante caminantes[max_caminantes];
	int num_caminantes = 0;
};
}
