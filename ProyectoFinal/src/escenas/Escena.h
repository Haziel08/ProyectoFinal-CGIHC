#pragma once
/*
 * Escena.h
 * Clase base de cada zona del proyecto: Espacio, Planeta Naranja,
 * Planeta Morado y Planeta Azul.
 *
 * Cada escena tiene lo SUYO:
 *   - su skybox      (assets/<escena>/skybox/)
 *   - su cámara      (cada escena recuerda dónde quedó su cámara)
 *   - sus luces
 *   - sus modelos, texturas y animaciones (cuando se agreguen)
 *
 * main.cpp solo elige qué escena está activa y la dibuja, así cada quien
 * trabaja en su carpeta src/escenas/<escena>/ sin tocar la de los demás.
 */
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
#include "SpotLight.h"

// Ubicaciones de los uniforms del shader de iluminación (las llena main.cpp)
struct ContextoRender
{
	GLuint uModel = 0;
	GLuint uColor = 0;
	GLuint uSpecularIntensity = 0;
	GLuint uShininess = 0;
};

class Escena
{
public:
	virtual ~Escena() {}

	// Nombre para el título de la ventana
	virtual const char* Nombre() const = 0;

	// Se llama UNA vez al inicio: skybox, cámara, luces, modelos, texturas.
	virtual void Inicializar() = 0;

	// Se llama cada frame mientras la escena está activa (animaciones, teclas propias).
	// dt = segundos desde el frame anterior.
	virtual void Actualizar(float dt, Window& ventana) {}

	// Dibujar los objetos de la escena (el shader ya está activo).
	virtual void Dibujar(const ContextoRender& ctx) {}

	// ---- Comunes a todas las escenas ----
	Camera& ObtenerCamara() { return camara; }
	void DibujarSkybox(const glm::mat4& vista, const glm::mat4& proyeccion);
	void AplicarLuces(Shader& shader);

protected:
	// Carga los 6 lados de una carpeta con los nombres que se usaban en clase:
	// rt, lf, up, dn, bk, ft + extensión.  Ej: CargarSkybox("assets/espacio/skybox/");
	void CargarSkybox(const std::string& carpeta, const std::string& extension = ".jpg");

	Camera camara;
	Skybox skybox;
	bool tieneSkybox = false;

	// La direccional siempre debe existir (el shader la usa)
	DirectionalLight luzDireccional = DirectionalLight(1.0f, 1.0f, 1.0f, 0.3f, 0.6f, 0.0f, -1.0f, -0.5f);
	PointLight lucesPuntuales[MAX_POINT_LIGHTS];
	unsigned int numLucesPuntuales = 0;
	SpotLight lucesSpot[MAX_SPOT_LIGHTS];
	unsigned int numLucesSpot = 0;
};
