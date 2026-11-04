#pragma once
// Interfaz comun de las escenas y contexto de dibujo.
#include <string>
#include <glew.h>
#include <glm.hpp>

#include "Window.h"
#include "Camera.h"
#include "Shader_light.h"
#include "Skybox.h"
#include "CommonValues.h"
#include "DirectionalLight.h"
#include "PointLight.h"

// Ubicaciones de los uniforms del shader de iluminación (las llena main.cpp)
struct ContextoRender
{
	GLuint uModel = 0;
	GLuint uColor = 0;
	GLuint uSpecularIntensity = 0;
	GLuint uShininess = 0;

	// Camara del frame (para escenas que usan otro shader, p. ej. el agua)
	glm::mat4 vista = glm::mat4(1.0f);
	glm::mat4 proyeccion = glm::mat4(1.0f);
	glm::vec3 posCamara = glm::vec3(0.0f);
};

class Escena
{
public:
	virtual ~Escena() {}

	// Nombre para el título de la ventana
	virtual const char* Nombre() const = 0;

	// Inicializacion del skybox, camara, modelos y texturas.
	virtual void Inicializar() = 0;

	// Actualizacion de animaciones y estados de la escena activa.
	// dt = segundos desde el frame anterior.
	virtual void Actualizar(float dt, Window& ventana) {}

	// Dibujar los objetos de la escena (el shader ya está activo).
	virtual void Dibujar(const ContextoRender& ctx) {}

	// Luz direccional propia de la escena (nullptr = usar la de main.cpp)
	virtual DirectionalLight* ObtenerLuzDireccional() { return nullptr; }

	// Luces puntuales propias de la escena (0 = usar las de main.cpp)
	virtual PointLight* ObtenerLucesPuntuales() { return nullptr; }
	virtual unsigned int NumLucesPuntuales() { return 0; }

	// ---- Comunes a todas las escenas ----
	Camera& ObtenerCamara() { return camara; }
	void DibujarSkybox(const glm::mat4& vista, const glm::mat4& proyeccion);

protected:
	// Carga las seis caras del cubo:
	// rt, lf, up, dn, bk, ft + extensión.  Ej: CargarSkybox("assets/espacio/skybox/");
	void CargarSkybox(const std::string& carpeta, const std::string& extension = ".jpg");

	Camera camara;
	Skybox skybox;
	bool tieneSkybox = false;

};
