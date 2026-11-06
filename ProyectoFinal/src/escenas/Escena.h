#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Definiciones de Escena >>>>>>>>>>>>>>>>>>>>>>
// Interfaz comun de las escenas y contexto de dibujo.
#include <string>
#include <cmath>
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
	GLuint u_model = 0;
	GLuint u_color = 0;
	GLuint u_specular_intensity = 0;
	GLuint u_shininess = 0;

	// Camara del frame (para escenas que usan otro shader agua y fuego)
	glm::mat4 vista = glm::mat4(1.0f);
	glm::mat4 proyeccion = glm::mat4(1.0f);
	glm::vec3 pos_camara = glm::vec3(0.0f);
};

// Parametrizacion comun a los asteroides de superficie y del espacio.
namespace CurvasOrbitales
{
inline glm::vec3 ElipsePlana(float angulo, float semieje_x, float semieje_z)
{
    return glm::vec3(semieje_x * std::cos(angulo), 0, semieje_z * std::sin(angulo));
}
inline glm::vec3 TangenteElipse(float angulo, float semieje_x, float semieje_z)
{
    return glm::vec3(-semieje_x * std::sin(angulo), 0, semieje_z * std::cos(angulo));
}
}

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
	virtual void Actualizar(float, Window&) {}

	// Dibujar los objetos de la escena (el shader ya está activo).
	virtual void Dibujar(const ContextoRender&) {}

	// ---- Comunes a todas las escenas ----
	Camera& ObtenerCamara() { return camara; }
	virtual float PlanoCercano() const { return 0.1f; }
	virtual float AlcanceVista() const { return 3500.0f; }

	// Luces propias de la escena
	virtual DirectionalLight* ObtenerLuzDireccional() { return nullptr; }
	virtual PointLight* ObtenerLucesPuntuales() { return nullptr; }
	virtual unsigned int NumLucesPuntuales() { return 0; }
	void DibujarSkybox(const glm::mat4& vista, const glm::mat4& proyeccion);

protected:
	// Carga las seis caras del cubo:
	// textura_derecha, textura_izquierda, textura_abajo, textura_arriba, textura_atras y textura_frente.
	void CargarSkybox(const std::string& carpeta, const std::string& extension = ".jpg", const std::string& prefijo = "textura_");

	Camera camara;
	Skybox skybox;
	bool tiene_skybox = false;

};
