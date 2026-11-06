#include "CicloDiaNoche.h"

#include <cmath>

void CicloDiaNoche::Actualizar(float dt)
{
	tiempo += dt;
	if (tiempo > duracion)
		tiempo -= duracion;     // vuelve a empezar el dia
}

float CicloDiaNoche::FactorDia() const
{
	// Coseno empieza en 1 (mediodia), baja a 0 a la mitad (medianoche) y regresa
	float angulo = 2.0f * 3.14159265f * tiempo / duracion;
	return 0.5f + 0.5f * cosf(angulo);
}

DatosLuz CicloDiaNoche::Luz() const
{
	float f = FactorDia();

	// Direccion desde el sol del skybox (hacia +Z y arriba) hacia la escena
	float inc = glm::radians(inclinacionSol);
	glm::vec3 haciaElSol(0.0f, sinf(inc), cosf(inc));

	DatosLuz luz;
	luz.direccion = -haciaElSol;
	luz.color = glm::mix(glm::vec3(0.45f, 0.55f, 0.85f), glm::vec3(1.0f), f);   // luna -> blanco
	luz.ambiental = glm::mix(0.12f, 0.40f, f);
	luz.difusa = glm::mix(0.15f, 0.85f, f);
	return luz;
}

DirectionalLight CicloDiaNoche::LuzDireccional() const
{
	DatosLuz l = Luz();
	return DirectionalLight(l.color.r, l.color.g, l.color.b,
		l.ambiental, l.difusa,
		l.direccion.x, l.direccion.y, l.direccion.z);
}
