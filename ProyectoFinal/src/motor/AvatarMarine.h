#pragma once
#include "Escena.h"
#include "ModeloAnimado.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"

//>>>>>>>>>>>>>>>>>>>>>> AvatarMarine >>>>>>>>>>>>>>>>>>>>>>
// Avatar del proyecto: el mismo marine aparece en las 4 escenas.
// Se carga una sola vez en main y cada escena dice por donde camina (CaminoAvatar).
// Tecla M: la camara lo sigue por detras.

class AvatarMarine
{
public:
	void Cargar();
	void Actualizar(float dt);
	void Dibujar(const ContextoRender& ctx, const Escena& escena, DirectionalLight* luz,
		PointLight* luces, unsigned int num_luces, SpotLight* focos, unsigned int num_focos);

	// Camara en tercera persona detras del avatar
	bool Camara(const Escena& escena, glm::vec3& objetivo, glm::vec3& rumbo, float& distancia, float& altura) const;

private:
	ModeloAnimado modelo;
	Shader shader;
	bool cargado = false;
	float tiempo = 0.0f;
};
