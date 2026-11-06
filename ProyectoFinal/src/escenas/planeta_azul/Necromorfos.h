#pragma once
// Necromorfos: NPCs con animacion esqueletal (modelo y animaciones de Mixamo).
//   - Uno por choza: cada uno camina en un circulo pequeno frente a su choza.
//   - Al prender la fogata (tecla F) los que tienen "bailaConFogata" se
//     detienen, voltean al fuego y bailan; al apagarla vuelven a caminar.
// El recorrido en circulo es por codigo:  x = cx + R cos(t),  z = cz + R sen(t)
// y miran hacia donde avanzan con giro = atan2(dx, dz).
#include "Escena.h"
#include "ModeloAnimado.h"
#include "Terreno.h"
#include "Aldea.h"

class Necromorfos
{
public:
	void Crear(const Terreno& terreno, const Aldea& aldea);
	void Actualizar(float dt, bool fogataEncendida);
	void Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int numLuces);

private:
	// Un necromorfo que da vueltas a un circulo
	struct Caminante
	{
		glm::vec2 centro;
		float radio;
		float angulo;        // donde va en el circulo (radianes)
		float velocidad;     // radianes por segundo (negativa = al reves)
		bool bailaConFogata;
	};

	void DibujarUno(const Caminante& c, int animacion, float tiempoAnimacion, GLuint uModel);

	const Terreno* terreno = nullptr;
	glm::vec2 fogata;
	bool bailando = false;
	float tiempo = 0.0f;

	ModeloAnimado necromorfo;
	int animCaminar = 0;
	int animBailar = 1;
	Shader shader;              // shader_esqueleto.vert + shader_light.frag

	static const int maxCaminantes = 10;
	Caminante caminantes[maxCaminantes];
	int numCaminantes = 0;
};
