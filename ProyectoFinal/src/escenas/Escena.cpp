#include "Escena.h"

#include <vector>

void Escena::CargarSkybox(const std::string& carpeta, const std::string& ext)
{
	// Orden de las caras para la direccion de muestreo del shader.
	std::vector<std::string> caras = {
		carpeta + "rt" + ext,
		carpeta + "lf" + ext,
		carpeta + "dn" + ext,
		carpeta + "up" + ext,
		carpeta + "bk" + ext,
		carpeta + "ft" + ext
	};
	skybox = Skybox(caras);
	tieneSkybox = true;
}

void Escena::DibujarSkybox(const glm::mat4& vista, const glm::mat4& proyeccion)
{
	if (tieneSkybox)
		skybox.DrawSkybox(vista, proyeccion);
}

