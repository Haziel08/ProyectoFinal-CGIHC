#pragma once
// CicloDiaNoche: controla la UNICA luz direccional del planeta azul.
// - La luz viene del sol dibujado en el skybox (lado Z+, inclinado hacia abajo).
// - De dia es blanca y fuerte; de noche es azulada y debil (luz de luna).
// - Un dia completo dura "duracion" segundos (maximo 2 minutos, lineamiento).
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
	float inclinacionSol = 22.0f;   // grados sobre el horizonte (igual que el sol del skybox)

	void Actualizar(float dt);

	float FactorDia() const;        // 1 = mediodia, 0 = medianoche
	bool EsDeNoche() const { return FactorDia() < 0.5f; }   // para prender lamparas

	DatosLuz Luz() const;
	DirectionalLight LuzDireccional() const;   // lista para shader.SetDirectionalLight

private:
	float tiempo = 0.0f;
};
