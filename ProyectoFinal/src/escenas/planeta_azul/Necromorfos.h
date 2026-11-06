#pragma once
// Necromorfos: NPCs con animacion esqueletal (modelo y animaciones de Mixamo).
//   - 2 en la aldea: con la fogata apagada caminan en circulo alrededor de ella;
//     al prender la fogata (tecla F) se detienen, voltean al fuego y bailan.
//   - 2 en la zona de criaturas: caminan en circulo todo el tiempo.
// El recorrido en circulo es por codigo:  x = cx + R cos(t),  z = cz + R sen(t)
// y miran hacia donde avanzan con giro = atan2(dx, dz).
#include "Escena.h"
#include "ModeloAnimado.h"
#include "Terreno.h"

class Necromorfos
{
public:
	void Crear(const Terreno& terreno, glm::vec2 centroFogata);
	void Actualizar(float dt, bool fogataEncendida);
	void Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int numLuces);

private:
	// Un necromorfo que da vueltas a un circulo
	struct Caminante
	{
		glm::vec2 centro;
		float radio;
		float angulo;        // donde va en el circulo (radianes)
		float velocidad;     // radianes por segundo
		bool bailaConFogata; // los de la aldea bailan cuando se prende la fogata
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

	static const int numCaminantes = 4;
	Caminante caminantes[numCaminantes];
};
