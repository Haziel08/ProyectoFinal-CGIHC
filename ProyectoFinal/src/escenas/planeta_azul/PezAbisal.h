#pragma once
#include "Escena.h"
#include "Model.h"
#include "Texture.h"
#include "SpotLight.h"
#include "Mesh_tn.h"
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cmath>
#include "Terreno.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> PezAbisal >>>>>>>>>>>>>>>>>>>>>>
// Pez abisal: nada cerca del fondo del lago con su farol encendido.
// Modelo jerarquico: cuerpo -> 2 aletas y el farol de la antena.
// Tecla L: prende / apaga el reflector (spotlight) del farol.

class PezAbisal
{
public:
	void Crear(const Terreno& terreno);
	void Actualizar(float dt, Window& ventana);
	void Dibujar(const ContextoRender& ctx);
	void DibujarHalo(const ContextoRender& ctx);   // brillo del farol (transparente, va al final)

	SpotLight foco;              // reflector que sale del farol
	bool linterna = true;        // empieza prendida

private:
	glm::mat4 MatrizCuerpo() const;

	const Terreno* terreno = nullptr;
	float tiempo = 0.0f;
	float angulo = 0.0f;
	glm::vec3 posicion{ 0.0f };
	float giro = 0.0f;

	Model cuerpo;
	Model aleta_izq;
	Model aleta_der;
	Model farol;
	Texture halo;
	MeshModel plano_halo;
};
}
