#include "Escena.h"

#include <vector>

void Escena::CargarSkybox(const std::string& carpeta, const std::string& ext)
{
	// Mismo orden que en las prácticas (rt, lf, dn, up, bk, ft), así un skybox
	// descargado que funcionaba en clase funciona igual aquí.
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

void Escena::AplicarLuces(Shader& shader)
{
	shader.SetDirectionalLight(&luzDireccional);
	shader.SetPointLights(lucesPuntuales, numLucesPuntuales);
	shader.SetSpotLights(lucesSpot, numLucesSpot);
}
