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

class Model;
class Texture;

// Ubicaciones de los uniforms del shader de iluminación (las llena main.cpp)
struct ContextoRender
{
    GLuint u_model = 0;
    GLuint u_color = 0;
    GLuint u_specular_intensity = 0;
    GLuint u_shininess = 0;
    glm::mat4 vista{1.0f}, proyeccion{1.0f};
    glm::vec3 pos_camara{0.0f};
};

// Ciclo compartido por ambos planetas; mediodia al iniciar y noche entre 30 y 90 s.
namespace IluminacionAmbiental
{
inline constexpr float duracion_dia = 120.0f;
inline float FactorDia(float tiempo, float duracion = duracion_dia)
{
    return 0.5f + 0.5f * std::cos(2.0f * 3.142f * tiempo / duracion);
}
} // namespace IluminacionAmbiental

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
} // namespace CurvasOrbitales

// Camino del avatar: da vueltas a un circulo a velocidad constante
// tiempo -> angulo = distancia / radio; el rumbo es la tangente del circulo
inline void CirculoAvatar(float tiempo, glm::vec3 centro, float radio, float velocidad,
                          glm::vec3& posicion, glm::vec3& rumbo)
{
    float angulo = tiempo * velocidad / radio;
    posicion = centro + glm::vec3(radio * std::cos(angulo), 0, radio * std::sin(angulo));
    rumbo = glm::vec3(-std::sin(angulo), 0, std::cos(angulo));
}

class Escena
{
public:
    virtual ~Escena()
    {
        delete malla_brillos;
    }

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
    Camera& ObtenerCamara()
    {
        return camara;
    }
    virtual DirectionalLight* ObtenerLuzDireccional()
    {
        return nullptr;
    }
    virtual PointLight* ObtenerLucesPuntuales()
    {
        return nullptr;
    }
    virtual unsigned int NumLucesPuntuales()
    {
        return 0;
    }
    // Reflectores (spotlights) propios de la escena
    virtual SpotLight* ObtenerFocos()
    {
        return nullptr;
    }
    virtual unsigned int NumFocos()
    {
        return 0;
    }
    virtual bool ObjetivoCamara(int, glm::vec3&, glm::vec3&, float&, float&) const { return false; }
    // Donde camina el avatar (marine) en esta escena: posicion, hacia donde mira y su estatura
    virtual bool CaminoAvatar(float, glm::vec3&, glm::vec3&, float&) const { return false; }
    virtual float PlanoCercano() const
    {
        return 0.1f;
    }
    virtual float AlcanceVista() const
    {
        return 3500.0f;
    }
    void DibujarSkybox(const glm::mat4& vista, const glm::mat4& proyeccion);

protected:
    // Modelo -> centro LED local -> halo que mira a la camara; no crea luces reales.
    void DibujarBrillos(const ContextoRender& ctx, const Model& modelo, const glm::mat4& padre,
                        Texture& textura, float intensidad = .65f);
    MeshModel* malla_brillos = nullptr;

    // Carga las seis caras del cubo:
    // textura_derecha, textura_izquierda, textura_abajo, textura_arriba, textura_atras y textura_frente.
    void CargarSkybox(const std::string& carpeta, const std::string& extension = ".jpg",
                      const std::string& prefijo = "textura_");

    Camera camara;
    Skybox skybox;
    bool tiene_skybox = false;
};
