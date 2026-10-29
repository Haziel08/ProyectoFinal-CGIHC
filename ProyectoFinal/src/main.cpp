/* Proyecto Final: escenas planetarias y animacion de una nave de carreras. */
#define STB_IMAGE_IMPLEMENTATION

//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y cabeceras >>>>>>>>>>>>>>>>>>>>>
#include <cstdio>
#include <vector>
#include <string>
#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "Window.h"
#include "Shader_light.h"
#include "Camera.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Escena.h"
#include "espacio/EscenaEspacio.h"
#include "planeta_naranja/EscenaPlanetaNaranja.h"
#include "planeta_morado/EscenaPlanetaMorado.h"
#include "planeta_azul/EscenaPlanetaAzul.h"

//>>>>>>>>>>>>>>>>>>>>>> Variables de iluminacion >>>>>>>>>>>>>>>>>>>>
namespace
{
DirectionalLight luzGeneral;
DirectionalLight luzPlanetaNaranja;
PointLight lucesPuntuales[MAX_POINT_LIGHTS];
SpotLight lucesSpot[MAX_SPOT_LIGHTS];
unsigned int numLucesPuntuales = 0;
unsigned int numLucesSpot = 0;

//>>>>>>>>>>>>>>>>>>>>>> Creacion de luces >>>>>>>>>>>>>>>>>>>>>>>>>>>
void CrearLuces()
{
    luzGeneral = DirectionalLight(1, 1, 1, 0.3f, 0.6f, 0, -1, -0.5f);
    luzPlanetaNaranja = DirectionalLight(1.0f, 0.79f, 0.60f,
        0.48f, 0.85f, -0.45f, 0.85f, -0.25f);
}

//>>>>>>>>>>>>>>>>>>>>>> Luz puntual de la nave >>>>>>>>>>>>>>>>>>>>>>
PointLight CrearLuzNave(const glm::vec3& p, float escala)
{
    return PointLight(1, 0, 0, 0, 3, p.x, p.y, p.z,
        1, 0.045f / escala, 0.0075f / (escala * escala));
}

//>>>>>>>>>>>>>>>>>>>>>> Luces de los motores >>>>>>>>>>>>>>>>>>>>>>>>
PointLight CrearLuzMotor(const glm::vec3& p, float escala)
{
    return PointLight(0.1f, 0.3f, 1, 0, 4, p.x, p.y, p.z,
        1, 0.09f / escala, 0.032f / (escala * escala));
}

//>>>>>>>>>>>>>>>>>>>>>> Faro inferior de la nave >>>>>>>>>>>>>>>>>>>>
SpotLight CrearFaroNave(const glm::vec3& p, const glm::vec3& d, float escala)
{
    return SpotLight(0.5f, 0.8f, 0, 0, 3, p.x, p.y, p.z, d.x, d.y, d.z,
        1, 0.045f / escala, 0.0075f / (escala * escala), 30);
}

//>>>>>>>>>>>>>>>>>>>>>> Actualizacion de luces >>>>>>>>>>>>>>>>>>>>>>
void ActualizarLucesNave(const glm::mat4& modeloNave, const Window& ventana)
{
    const float escala = glm::length(glm::vec3(modeloNave[0]));
    if (ventana.getLuzNaveEncendida())
    {
        glm::vec3 p(modeloNave * glm::vec4(0, 2.7f, 0, 1));
        lucesPuntuales[numLucesPuntuales++] = CrearLuzNave(p, escala);
        p = glm::vec3(modeloNave * glm::vec4(0, -0.5f, 0, 1));
        glm::vec3 d = glm::normalize(glm::vec3(modeloNave * glm::vec4(0, -1, 0, 0)));
        lucesSpot[numLucesSpot++] = CrearFaroNave(p, d, escala);
    }
    for (int i = 0; i < 2; ++i)
    {
        if (!(i == 0 ? ventana.getMotor1Encendido() : ventana.getMotor2Encendido()))
            continue;
        glm::vec3 p(modeloNave * glm::vec4(15.7f, 0, i == 0 ? 4.0f : -4.0f, 1));
        lucesPuntuales[numLucesPuntuales++] = CrearLuzMotor(p, escala);
    }
}
}

//>>>>>>>>>>>>>>>>>>>>>> Programa principal >>>>>>>>>>>>>>>>>>>>>>>>>
int main()
{
    //>>>>>>>>>>>>>>>>>>>>>> Ventana y shaders >>>>>>>>>>>>>>>>>>>>>>>>>>>
    Window ventana(1366, 768);
    if (ventana.Initialise() != 0) return 1;
    Shader shader;
    shader.CreateFromFiles("shaders/shader_light.vert", "shaders/shader_light.frag");

    //>>>>>>>>>>>>>>>>>>>>>> Inicializacion de escenas >>>>>>>>>>>>>>>>>>>
    EscenaPlanetaNaranja* planetaNaranja = new EscenaPlanetaNaranja();
    std::vector<Escena*> escenas = {
        new EscenaEspacio(), planetaNaranja,
        new EscenaPlanetaMorado(), new EscenaPlanetaAzul()
    };
    for (Escena* escena : escenas) escena->Inicializar();
    CrearLuces();
    int escenaActual = ventana.getEscenaActiva();
    ventana.setTitulo((std::string("Proyecto Final CGeIHC  |  ") + escenas[escenaActual]->Nombre()).c_str());
    GLfloat ultimoTiempo = static_cast<GLfloat>(glfwGetTime());

    //>>>>>>>>>>>>>>>>>>>>>> Ciclo de eventos y dibujo >>>>>>>>>>>>>>>>>>>
    while (!ventana.getShouldClose())
    {
        //>>>>>>>>>>>>>>>>>>>>>> Tiempo de animacion >>>>>>>>>>>>>>>>>>>>>>>>>
        GLfloat ahora = static_cast<GLfloat>(glfwGetTime());
        GLfloat dt = glm::clamp(ahora - ultimoTiempo, 0.0f, 0.1f);
        ultimoTiempo = ahora;

        //>>>>>>>>>>>>>>>>>>>>>> Eventos de ventana y teclado >>>>>>>>>>>>>>>>
        ventana.NuevoFrame();
        glfwPollEvents();
        int seleccion = ventana.getEscenaActiva();
        if (seleccion != escenaActual)
        {
            escenaActual = seleccion;
            ventana.setTitulo((std::string("Proyecto Final CGeIHC  |  ") + escenas[escenaActual]->Nombre()).c_str());
        }

        //>>>>>>>>>>>>>>>>>>>>>> Animacion de la escena >>>>>>>>>>>>>>>>>>>>>>
        Escena* escena = escenas[escenaActual];
        escena->Actualizar(dt, ventana);

        //>>>>>>>>>>>>>>>>>>>>>> Camara y proyeccion >>>>>>>>>>>>>>>>>>>>>>>>>
        Camera& camara = escena->ObtenerCamara();
        camara.keyControl(ventana.getsKeys(), dt);
        camara.mouseControl(ventana.getXChange(), ventana.getYChange());
        glm::mat4 vista = camara.calculateViewMatrix();
        glm::vec3 ojo = camara.getCameraPosition();
        glm::mat4 proyeccion = glm::perspective(glm::radians(60.0f),
            ventana.getBufferWidth() / ventana.getBufferHeight(), 0.1f, 2000.0f);

        //>>>>>>>>>>>>>>>>>>>>>> Luces de la escena activa >>>>>>>>>>>>>>>>>>>
        numLucesPuntuales = numLucesSpot = 0;
        if (escenaActual == 1)
            ActualizarLucesNave(planetaNaranja->ObtenerMatrizNave(), ventana);

        //>>>>>>>>>>>>>>>>>>>>>> Skybox >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        escena->DibujarSkybox(vista, proyeccion);

        //>>>>>>>>>>>>>>>>>>>>>> Uniforms de iluminacion >>>>>>>>>>>>>>>>>>>>
        shader.UseShader();
        glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(proyeccion));
        glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(vista));
        glUniform3f(shader.GetEyePositionLocation(), ojo.x, ojo.y, ojo.z);
        shader.SetDirectionalLight(escenaActual == 1 ? &luzPlanetaNaranja : &luzGeneral);
        shader.SetPointLights(lucesPuntuales, numLucesPuntuales);
        shader.SetSpotLights(lucesSpot, numLucesSpot);

        //>>>>>>>>>>>>>>>>>>>>>> Modelos de la escena >>>>>>>>>>>>>>>>>>>>>>>>
        ContextoRender ctx;
        ctx.uModel = shader.GetModelLocation();
        ctx.uColor = shader.getColorLocation();
        ctx.uSpecularIntensity = shader.GetSpecularIntensityLocation();
        ctx.uShininess = shader.GetShininessLocation();
        escena->Dibujar(ctx);

        glUseProgram(0);
        ventana.swapBuffers();
    }

    //>>>>>>>>>>>>>>>>>>>>>> Liberacion de escenas >>>>>>>>>>>>>>>>>>>>>>>
    for (Escena* escena : escenas) delete escena;
    return 0;
}
