/* Proyecto Final: escenas planetarias y animacion de una nave de carreras. */
#define STB_IMAGE_IMPLEMENTATION

//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y cabeceras >>>>>>>>>>>>>>>>>>>>>
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

//>>>>>>>>>>>>>>>>>>>>>> Tipos y funciones de iluminacion >>>>>>>>>>>>
namespace
{
// Se instancia en main; estas funciones reciben el estado que modifican.
struct LucesNaves
{
    PointLight puntuales[max_point_lights];
    SpotLight focos[max_spot_lights];
    unsigned int cantidad_puntuales = 0;
    unsigned int cantidad_focos = 0;
};

//>>>>>>>>>>>>>>>>>>>>>> Luz puntual de la nave >>>>>>>>>>>>>>>>>>>>>>
PointLight CrearLuzNave(const glm::vec3& p, float escala)
{
    return PointLight(1, 0, 0, 0, 3, p.x, p.y, p.z, 1, 0.045f / escala,
                      (0.015f / 2.0f) / (escala * escala));
}

//>>>>>>>>>>>>>>>>>>>>>> Luces de los motores >>>>>>>>>>>>>>>>>>>>>>>>
PointLight CrearLuzMotor(const glm::vec3& p, float escala)
{
    return PointLight(0.1f, 0.3f, 1, 0, 4, p.x, p.y, p.z, 1, 0.09f / escala, 0.032f / (escala * escala));
}

//>>>>>>>>>>>>>>>>>>>>>> Faro inferior de la nave >>>>>>>>>>>>>>>>>>>>
SpotLight CrearFaroNave(const glm::vec3& p, const glm::vec3& d, float escala)
{
    return SpotLight(0.5f, 0.8f, 0, 0, 3, p.x, p.y, p.z, d.x, d.y, d.z, 1, 0.045f / escala,
                     (0.015f / 2.0f) / (escala * escala), 30);
}

//>>>>>>>>>>>>>>>>>>>>>> Actualizacion de luces >>>>>>>>>>>>>>>>>>>>>>
void ActualizarLucesNave(const glm::mat4& modelo_nave, const Window& ventana, LucesNaves& luces)
{
    const float escala = glm::length(glm::vec3(modelo_nave[0]));
    if (ventana.getLuzNaveEncendida())
    {
        // Nave -> luz del casco y faro inferior: offsets locales junto a su padre.
        luces.puntuales[luces.cantidad_puntuales++] =
            CrearLuzNave(glm::vec3(modelo_nave * glm::vec4(0, 2.7f, 0, 1)), escala);
        luces.focos[luces.cantidad_focos++] =
            CrearFaroNave(glm::vec3(modelo_nave * glm::vec4(0, -0.5f, 0, 1)),
                          glm::normalize(glm::vec3(modelo_nave * glm::vec4(0, -1, 0, 0))), escala);
    }
    for (int i = 0; i < 2; ++i)
    {
        if (!(i == 0 ? ventana.getMotor1Encendido() : ventana.getMotor2Encendido()))
            continue;
        // Nave -> motor izquierdo/derecho.
        luces.puntuales[luces.cantidad_puntuales++] =
            CrearLuzMotor(glm::vec3(modelo_nave * glm::vec4(15.7f, 0, i == 0 ? 4.0f : -4.0f, 1)), escala);
    }
}
} // namespace

//>>>>>>>>>>>>>>>>>>>>>> Programa principal >>>>>>>>>>>>>>>>>>>>>>>>>
int main()
{
    //>>>>>>>>>>>>>>>>>>>>>> Ventana y shaders >>>>>>>>>>>>>>>>>>>>>>>>>>>
    Window ventana(1366, 768);
    if (ventana.Initialise() != 0)
        return 1;
    Shader shader;
    shader.CreateFromFiles("shaders/shader_light.vert", "shaders/shader_light.frag");

    //>>>>>>>>>>>>>>>>>>>>>> Inicializacion de escenas >>>>>>>>>>>>>>>>>>>
    EscenaPlanetaNaranja* planeta_naranja = new EscenaPlanetaNaranja();
    std::vector<Escena*> escenas = {new EscenaEspacio(), planeta_naranja, new EscenaPlanetaMorado(),
                                    new EscenaPlanetaAzul()};
    for (Escena* escena : escenas)
        escena->Inicializar();
    //>>>>>>>>>>>>>>>>>>>>>> Luces y estado local del programa >>>>>>>>>>>
    DirectionalLight luz_general(1, 1, 1, 0.3f, 0.6f, 0, -1, -0.5f);
    DirectionalLight luz_planeta_naranja(1.0f, 0.79f, 0.60f, 0.48f, 0.85f, -0.45f, 0.85f, -0.25f);
    LucesNaves luces;

    // Las ubicaciones del shader no cambian durante el ciclo.
    ContextoRender ctx;
    ctx.u_model = shader.GetModelLocation();
    ctx.u_color = shader.getColorLocation();
    ctx.u_specular_intensity = shader.GetSpecularIntensityLocation();
    ctx.u_shininess = shader.GetShininessLocation();
    int escena_actual = ventana.getEscenaActiva();
    ventana.setTitulo(
        (std::string("Proyecto Final CGeIHC  |  ") + escenas[escena_actual]->Nombre()).c_str());
    GLfloat ultimo_tiempo = static_cast<GLfloat>(glfwGetTime());

    //>>>>>>>>>>>>>>>>>>>>>> Ciclo de eventos y dibujo >>>>>>>>>>>>>>>>>>>
    while (!ventana.getShouldClose())
    {
        //>>>>>>>>>>>>>>>>>>>>>> Tiempo de animacion >>>>>>>>>>>>>>>>>>>>>>>>>
        GLfloat ahora = static_cast<GLfloat>(glfwGetTime());
        GLfloat dt = glm::clamp(ahora - ultimo_tiempo, 0.0f, 0.1f);
        ultimo_tiempo = ahora;

        //>>>>>>>>>>>>>>>>>>>>>> Eventos de ventana y teclado >>>>>>>>>>>>>>>>
        ventana.NuevoFrame();
        glfwPollEvents();
        int seleccion = ventana.getEscenaActiva();
        if (seleccion != escena_actual)
        {
            escena_actual = seleccion;
            ventana.setTitulo(
                (std::string("Proyecto Final CGeIHC  |  ") + escenas[escena_actual]->Nombre()).c_str());
        }

        //>>>>>>>>>>>>>>>>>>>>>> Animacion de la escena >>>>>>>>>>>>>>>>>>>>>>
        Escena* escena = escenas[escena_actual];
        escena->Actualizar(dt, ventana);

        //>>>>>>>>>>>>>>>>>>>>>> Camara y proyeccion >>>>>>>>>>>>>>>>>>>>>>>>>
        Camera& camara = escena->ObtenerCamara();
        camara.keyControl(ventana.getsKeys(), dt);
        camara.mouseControl(ventana.getXChange(), ventana.getYChange());
        glm::mat4 vista = camara.calculateViewMatrix();
        glm::vec3 ojo = camara.getCameraPosition();
        glm::mat4 proyeccion =
            glm::perspective(glm::radians(60.0f), ventana.getBufferWidth() / ventana.getBufferHeight(),
                             escena->PlanoCercano(), escena->AlcanceVista());

        //>>>>>>>>>>>>>>>>>>>>>> Luces de la escena activa >>>>>>>>>>>>>>>>>>>
        luces.cantidad_puntuales = luces.cantidad_focos = 0;
        if (escena_actual == 1)
        {
            ActualizarLucesNave(planeta_naranja->ObtenerMatrizNave(), ventana, luces);
            ActualizarLucesNave(planeta_naranja->ObtenerMatrizNaveEspiral(), ventana, luces);
            for (int i = 0; i < 9; ++i)
            {
                glm::vec3 posicion = planeta_naranja->ObtenerPosicionLuzPoste(i, ojo);
                luces.puntuales[luces.cantidad_puntuales++] = PointLight(
                    1, 0.88f, 0.65f, 0, 1.2f, posicion.x, posicion.y, posicion.z, 1, 0.12f, 0.025f);
            }
        }

        // Luces propias de la escena activa (p. ej. fogata y antorchas del planeta azul)
        for (unsigned int i = 0; i < escena->NumLucesPuntuales() && luces.cantidad_puntuales < max_point_lights; ++i)
            luces.puntuales[luces.cantidad_puntuales++] = escena->ObtenerLucesPuntuales()[i];

        //>>>>>>>>>>>>>>>>>>>>>> Skybox >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        escena->DibujarSkybox(vista, proyeccion);

        //>>>>>>>>>>>>>>>>>>>>>> Uniforms de iluminacion >>>>>>>>>>>>>>>>>>>>
        shader.UseShader();
        glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(proyeccion));
        glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(vista));
        glUniform3f(shader.GetEyePositionLocation(), ojo.x, ojo.y, ojo.z);
        DirectionalLight* luz_escena = escena->ObtenerLuzDireccional();   // la escena puede traer la suya
        if (luz_escena == nullptr)
            luz_escena = (escena_actual == 1) ? &luz_planeta_naranja : &luz_general;
        shader.SetDirectionalLight(luz_escena);
        shader.SetPointLights(luces.puntuales, luces.cantidad_puntuales);
        shader.SetSpotLights(luces.focos, luces.cantidad_focos);

        //>>>>>>>>>>>>>>>>>>>>>> Modelos de la escena >>>>>>>>>>>>>>>>>>>>>>>>
        ctx.vista = vista;              // para escenas con shaders propios (agua, fuego)
        ctx.proyeccion = proyeccion;
        ctx.pos_camara = ojo;
        escena->Dibujar(ctx);

        glUseProgram(0);
        ventana.swapBuffers();
    }

    //>>>>>>>>>>>>>>>>>>>>>> Liberacion de escenas >>>>>>>>>>>>>>>>>>>>>>>
    for (Escena* escena : escenas)
        delete escena;
    return 0;
}
