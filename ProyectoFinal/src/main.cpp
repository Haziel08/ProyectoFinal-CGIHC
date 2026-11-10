/* Proyecto Final: escenas planetarias y animacion de una nave de carreras. */
#define STB_IMAGE_IMPLEMENTATION

//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y cabeceras >>>>>>>>>>>>>>>>>>>>>
#include <algorithm>
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
#include "AvatarMarine.h"

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

SpotLight CrearFaroRival(const glm::mat4& marco)
{
    const glm::vec3 posicion(marco*glm::vec4(0,.3f,3.5f,1));
    const glm::vec3 rumbo = glm::normalize(glm::vec3(marco*glm::vec4(0,-.27f,1,0)));
    return SpotLight(.82f,.91f,1,0,3.2f,posicion.x,posicion.y,posicion.z,
        rumbo.x,rumbo.y,rumbo.z,1,.036f,.004f,26);
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
    {
        ventana.setTitulo((std::string("Proyecto Final | Cargando ") + escena->Nombre()).c_str());
        escena->Inicializar();
    }
    // El avatar se carga una vez y se usa en todas las escenas
    ventana.setTitulo("Proyecto Final | Cargando avatar");
    AvatarMarine avatar;
    avatar.Cargar();
    //>>>>>>>>>>>>>>>>>>>>>> Luces y estado local del programa >>>>>>>>>>>
    DirectionalLight luz_general(1, 1, 1, 0.3f, 0.6f, 0, -1, -0.5f);
    LucesNaves luces;

    // Las ubicaciones del shader no cambian durante el ciclo.
    ContextoRender ctx;
    ctx.u_model = shader.GetModelLocation();
    ctx.u_color = shader.getColorLocation();
    ctx.u_specular_intensity = shader.GetSpecularIntensityLocation();
    ctx.u_shininess = shader.GetShininessLocation();
    int camara_activa = 0;
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
            camara_activa = 0;
            ventana.setTitulo(
                (std::string("Proyecto Final CGeIHC  |  ") + escenas[escena_actual]->Nombre()).c_str());
        }

        //>>>>>>>>>>>>>>>>>>>>>> Animacion de la escena >>>>>>>>>>>>>>>>>>>>>>
        Escena* escena = escenas[escena_actual];
        escena->Actualizar(dt, ventana);
        avatar.Actualizar(dt);

        //>>>>>>>>>>>>>>>>>>>>>> Camara y proyeccion >>>>>>>>>>>>>>>>>>>>>>>>>
        Camera& camara = escena->ObtenerCamara();
        if (escena_actual == 2 &&
            (ventana.TeclaPresionada(GLFW_KEY_I) || ventana.TeclaPresionada(GLFW_KEY_0)))
            camara_activa = 6;
        if (ventana.TeclaPresionada(GLFW_KEY_5)) camara_activa = 0;
        if (escena_actual == 0 && ventana.TeclaPresionada(GLFW_KEY_6)) camara_activa = 1;
        if (escena_actual == 1 && ventana.TeclaPresionada(GLFW_KEY_7)) camara_activa = 2;
        if (escena_actual == 3 && ventana.TeclaPresionada(GLFW_KEY_8)) camara_activa = 3;
        if (escena_actual == 3 && ventana.TeclaPresionada(GLFW_KEY_9)) camara_activa = 4;
        if (escena_actual == 1 && ventana.TeclaPresionada(GLFW_KEY_0)) camara_activa = 5;
        if (ventana.TeclaPresionada(GLFW_KEY_M)) camara_activa = 9;     // camara del avatar (todas)

        glm::vec3 objetivo, rumbo;
        float distancia, altura;
        glm::mat4 vista;
        glm::vec3 ojo;
        const float raton_x = ventana.getXChange();
        const float raton_y = ventana.getYChange();
        const bool sigue_camara = camara_activa == 9
            ? avatar.Camara(*escena, objetivo, rumbo, distancia, altura)
            : escena->ObjetivoCamara(camara_activa, objetivo, rumbo, distancia, altura);
        if (sigue_camara)
        {
            rumbo.y = 0.0f;
            rumbo = glm::normalize(rumbo);
            ojo = objetivo - rumbo * distancia;
            ojo.y = altura;
            vista = glm::lookAt(ojo, objetivo + glm::vec3(0, distancia * .07f, 0), glm::vec3(0, 1, 0));
        }
        else
        {
            camara.keyControl(ventana.getsKeys(), dt);
            camara.mouseControl(raton_x, raton_y);
            vista = camara.calculateViewMatrix();
            ojo = camara.getCameraPosition();
        }
        glm::mat4 proyeccion =
            glm::perspective(glm::radians(60.0f), ventana.getBufferWidth() / ventana.getBufferHeight(),
                             escena->PlanoCercano(), escena->AlcanceVista());

        //>>>>>>>>>>>>>>>>>>>>>> Luces de la escena activa >>>>>>>>>>>>>>>>>>>
        luces.cantidad_puntuales = luces.cantidad_focos = 0;
        if (escena_actual == 1)
        {
            ActualizarLucesNave(planeta_naranja->ObtenerMatrizNave(), ventana, luces);
            ActualizarLucesNave(planeta_naranja->ObtenerMatrizNaveEspiral(), ventana, luces);
            luces.focos[luces.cantidad_focos++] = CrearFaroRival(planeta_naranja->MarcoFaroRival(false));
            luces.focos[luces.cantidad_focos++] = CrearFaroRival(planeta_naranja->MarcoFaroRival(true));
        }

        for (unsigned int i = 0;
             i < escena->NumLucesPuntuales() && luces.cantidad_puntuales < max_point_lights; ++i)
            luces.puntuales[luces.cantidad_puntuales++] = escena->ObtenerLucesPuntuales()[i];
        for (unsigned int i = 0; i < escena->NumFocos() && luces.cantidad_focos < max_spot_lights; ++i)
            luces.focos[luces.cantidad_focos++] = escena->ObtenerFocos()[i];

        //>>>>>>>>>>>>>>>>>>>>>> Skybox >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        escena->DibujarSkybox(vista, proyeccion);

        //>>>>>>>>>>>>>>>>>>>>>> Uniforms de iluminacion >>>>>>>>>>>>>>>>>>>>
        shader.UseShader();
        glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(proyeccion));
        glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(vista));
        glUniform3f(shader.GetEyePositionLocation(), ojo.x, ojo.y, ojo.z);
        DirectionalLight* luz_escena = escena->ObtenerLuzDireccional();
        shader.SetDirectionalLight(luz_escena ? luz_escena : &luz_general);
        shader.SetPointLights(luces.puntuales, luces.cantidad_puntuales);
        shader.SetSpotLights(luces.focos, luces.cantidad_focos);

        //>>>>>>>>>>>>>>>>>>>>>> Modelos de la escena >>>>>>>>>>>>>>>>>>>>>>>>
        ctx.vista = vista;
        ctx.proyeccion = proyeccion;
        ctx.pos_camara = ojo;
        escena->Dibujar(ctx);
        avatar.Dibujar(ctx, *escena, luz_escena ? luz_escena : &luz_general,
                       luces.puntuales, luces.cantidad_puntuales, luces.focos, luces.cantidad_focos);

        glUseProgram(0);
        ventana.swapBuffers();
    }

    //>>>>>>>>>>>>>>>>>>>>>> Liberacion de escenas >>>>>>>>>>>>>>>>>>>>>>>
    for (Escena* escena : escenas)
        delete escena;
    return 0;
}
