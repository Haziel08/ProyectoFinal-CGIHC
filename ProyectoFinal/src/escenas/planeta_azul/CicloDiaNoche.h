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

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> CicloDiaNoche >>>>>>>>>>>>>>>>>>>>>>
// CicloDiaNoche

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
	float duracion = IluminacionAmbiental::duracion_dia;        // segundos de un dia completo
	float inclinacion_sol = 22.0f;   // grados sobre el horizonte

	void Actualizar(float dt);

	float FactorDia() const;        // 1 = mediodia, 0 = medianoche
	bool EsDeNoche() const { return FactorDia() < 0.5f; }   // para prender lamparas

	DatosLuz Luz() const;
	DirectionalLight LuzDireccional() const;   // lista para shader.SetDirectionalLight

private:
	float tiempo = 0.0f;
};
}
