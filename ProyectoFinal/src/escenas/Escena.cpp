//>>>>>>>>>>>>>>>>>>>>>> Escena >>>>>>>>>>>>>>>>>>>>>>
#include "Escena.h"

#include <vector>

void Escena::CargarSkybox(const std::string& carpeta, const std::string& ext, const std::string& prefijo)
{
	// Orden de las caras para la direccion de muestreo del shader.
	std::vector<std::string> caras = {
		carpeta + prefijo + "derecha" + ext,
		carpeta + prefijo + "izquierda" + ext,
		carpeta + prefijo + "abajo" + ext,
		carpeta + prefijo + "arriba" + ext,
		carpeta + prefijo + "atras" + ext,
		carpeta + prefijo + "frente" + ext
	};
	skybox = Skybox(caras);
	tiene_skybox = true;
}

void Escena::DibujarSkybox(const glm::mat4& vista, const glm::mat4& proyeccion)
{
	if (tiene_skybox)
		skybox.DrawSkybox(vista, proyeccion);
}

