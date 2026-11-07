#pragma once
// CicloDiaNoche
#include <glm.hpp>
#include "DirectionalLight.h"

// Valores de la luz para mandarlos a cualquier shader (tambien al del agua)
struct DatosLuz
{
	glm::vec3 direccion;
	glm::vec3 color;
	float ambiental;
	float difusa;
};

class CicloDiaNoche
{
public:
	float duracion = 120.0f;        // segundos de un dia completo
	float inclinacionSol = 22.0f;   // grados sobre el horizonte
	glm::vec3 colorDia = glm::vec3(1.0f);                   // color de la luz a mediodia
	glm::vec3 colorNoche = glm::vec3(0.45f, 0.55f, 0.85f);  // color de la luz de la luna

	void Actualizar(float dt);

	float FactorDia() const;        // 1 = mediodia, 0 = medianoche
	bool EsDeNoche() const { return FactorDia() < 0.5f; }   // para prender lamparas

	DatosLuz Luz() const;
	DirectionalLight LuzDireccional() const;   // lista para shader.SetDirectionalLight

private:
	float tiempo = 0.0f;
};
