/* Proyecto Final: escenas planetarias y animacion de una nave de carreras. */
#define STB_IMAGE_IMPLEMENTATION

//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y cabeceras >>>>>>>>>>>>>>>>>>>>>
#include <algorithm>
#include <vector>
#include <string>
#include <array>
#include <fstream>
#include <sstream>
#include <gtc/constants.hpp>
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

//>>>>>>>>>>>>>>>>>>>>>> Patrullas: ruta -> esqueleto -> mano -> linterna >>>>>>>>
struct ClavePatrulla
{
    float segundo;
    glm::vec3 punto;
};

class PatrullasMarine
{
public:
    void Cargar()
    {
        if (!marine.Cargar("assets/comun/modelos/marine.fbx",
                           "assets/comun/texturas/marine_color.jpg", true)) return;
        companero_cargado = companero.Cargar("assets/comun/modelos/marine_companero.glb",
                                             "assets/comun/texturas/marine_companero.jpg",true);
        linterna.LoadModel("assets/comun/modelos/linterna.obj");
        shader.CreateFromString(PlanetaAzul::esqueleto_vert,
                                shader.ReadFile("shaders/shader_light.frag").c_str());
        const char* nombres[] = {"espacio", "planeta_naranja", "planeta_morado", "planeta_azul"};
        for (size_t i = 0; i < rutas.size(); ++i)
            rutas[i] = LeerRuta(std::string("src/escenas/") + nombres[i] + "/recorridos/marine_frames.txt");
        cargado = true;
    }

    void Actualizar(int indice, const Escena& escena, float dt, const glm::mat4& estacion)
    {
        escena_activa = indice;
        const auto& ruta = rutas[indice];
        activo = (indice == 0 ? companero_cargado : cargado) && !ruta.empty();
        if (!activo) return;

        const float dt_patrulla = indice == 0 ? dt * 6.0f : dt;
        tiempos[indice] = std::fmod(tiempos[indice] + dt_patrulla, ruta.back().segundo);
        const float escala[] = {2.625f, 16.0f, 17.0f, 4.8f};
        tamano = escala[indice];
        const float tiempo = tiempos[indice];
        const glm::vec3 punto = Evaluar(ruta, tiempo);
        const glm::vec3 tangente = Evaluar(ruta, tiempo + .35f) - Evaluar(ruta, tiempo - .35f);
        const float giro = std::atan2(tangente.x, tangente.z);
        glm::vec3 suelo = punto;
        suelo.y += escena.SueloPatrulla(punto.x, punto.z);
        const glm::mat4 raiz = glm::rotate(glm::translate(glm::mat4(1), suelo), giro, {0,1,0});
        marcha[indice] = std::fmod(marcha[indice] + dt, 8.0f);
        if (indice == 0) companero.CalcularPose(0, marcha[indice] * .75f);
        else marine.CalcularPose(0, marcha[indice] * (indice == 3 ? 1.25f : 1.0f));
        if (indice == 0)
        {
            for (size_t i = 0; i < matrices.size(); ++i)
                matrices[i] = estacion * glm::rotate(glm::mat4(1),
                    glm::two_pi<float>() * float(i) / float(matrices.size()), {0,1,0}) *
                    glm::scale(raiz, glm::vec3(tamano));
            tamano = glm::length(glm::vec3(matrices[0][2]));
        }
        else
            matrices[0] = glm::scale(raiz, glm::vec3(tamano));

        rumbo = glm::normalize(glm::vec3(matrices[0][2]));
        const glm::vec3 mano(matrices[0] * (indice == 0 ? companero.MarcoHueso("mixamorig:RightHand")
                                         : marine.MarcoHueso("RightHand")) *
                             glm::vec4(0,0,0,1));
        const glm::vec3 frente = glm::normalize(rumbo - glm::vec3(0,.14f,0));
        const glm::vec3 lado = glm::normalize(glm::cross(glm::vec3(0,1,0), frente));
        marco_linterna = glm::mat4(glm::vec4(lado,0), glm::vec4(glm::cross(frente,lado),0),
                                  glm::vec4(frente,0), glm::vec4(mano,1));
        marco_linterna = glm::scale(marco_linterna, glm::vec3(tamano * .50f));
    }

    void ActualizarCompanero(int indice, const Escena& escena, const glm::vec2& aldea, float dt)
    {
        companero_activo = companero_cargado && indice != 0;
        if (!companero_activo) return;

        const glm::vec2 centros[] = {{0,0},{-90,0},{0,0},aldea};
        const float radios[] = {0,455,175,50};
        const float estaturas[] = {0,7,16,2.2f};
        tiempo_companero += dt;
        estatura_companero = estaturas[indice];
        const float radio = radios[indice];
        const float angulo = tiempo_companero * estatura_companero * 1.1f / radio;
        const glm::vec2 centro = centros[indice];
        posicion_companero = {centro.x + radio * std::cos(angulo),0,centro.y + radio * std::sin(angulo)};
        posicion_companero.y = escena.SueloPatrulla(posicion_companero.x,posicion_companero.z);
        rumbo_companero = {-std::sin(angulo),0,std::cos(angulo)};
        companero.CalcularPose(0,tiempo_companero * .75f);
    }

    bool ObjetivoCompanero(glm::vec3& objetivo, glm::vec3& rumbo, float& distancia, float& altura) const
    {
        if (!companero_activo) return Objetivo(objetivo,rumbo,distancia,altura);
        objetivo = posicion_companero + glm::vec3(0,estatura_companero*.7f,0);
        rumbo = rumbo_companero;
        distancia = estatura_companero * 2.8f;
        altura = posicion_companero.y + estatura_companero * 1.4f;
        return true;
    }

    bool Objetivo(glm::vec3& objetivo, glm::vec3& frente, float& distancia, float& altura) const
    {
        if (!activo) return false;
        objetivo = glm::vec3(matrices[0][3]) + rumbo * (tamano * .72f) + glm::vec3(0,tamano*.58f,0);
        frente = rumbo;
        distancia = tamano * 2.45f;
        altura = objetivo.y + tamano * .76f;
        return true;
    }

    bool Activo() const { return activo; }

    SpotLight Luz() const
    {
        const glm::vec3 p(marco_linterna * glm::vec4(0,0,.20f,1));
        const glm::vec3 d = glm::normalize(glm::vec3(marco_linterna[2]));
        return SpotLight(1,.94f,.82f,0,4.2f,p.x,p.y,p.z,d.x,d.y,d.z,1,.12f/tamano,
                         .26f/(tamano*tamano), 31);
    }

    void Dibujar(Shader& principal, const ContextoRender& ctx,
                 DirectionalLight& sol, LucesNaves& luces)
    {
        if (!activo) return;
        principal.UseShader();
        GLint programa = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
        glUniform1i(glGetUniformLocation(programa,"modo_efecto"),0);
        glUniform3f(ctx.u_color,1,1,1);
        glUniformMatrix4fv(ctx.u_model,1,GL_FALSE,glm::value_ptr(marco_linterna));
        linterna.RenderModel();

        shader.UseShader();
        glGetIntegerv(GL_CURRENT_PROGRAM,&programa);
        shader.SetDirectionalLight(&sol);
        shader.SetPointLights(luces.puntuales,luces.cantidad_puntuales);
        shader.SetSpotLights(luces.focos,luces.cantidad_focos);
        glUniformMatrix4fv(shader.GetViewLocation(),1,GL_FALSE,glm::value_ptr(ctx.vista));
        glUniformMatrix4fv(shader.GetProjectionLocation(),1,GL_FALSE,glm::value_ptr(ctx.proyeccion));
        glUniform3fv(shader.GetEyePositionLocation(),1,glm::value_ptr(ctx.pos_camara));
        glUniform3f(shader.getColorLocation(),1,1,1);
        glUniform1f(shader.GetSpecularIntensityLocation(),.16f);
        glUniform1f(shader.GetShininessLocation(),16);
        auto& patrulla = escena_activa == 0 ? companero : marine;
        for (size_t i = 0, cantidad = escena_activa == 0 ? matrices.size() : 1; i < cantidad; ++i)
        {
            const glm::mat4 cuerpo = matrices[i] * patrulla.Normalizacion();
            glUniformMatrix4fv(shader.GetModelLocation(),1,GL_FALSE,glm::value_ptr(cuerpo));
            patrulla.Dibujar(GLuint(programa));
        }
        if (companero_activo)
        {
            const glm::mat4 raiz = glm::rotate(glm::translate(glm::mat4(1),posicion_companero),
                std::atan2(rumbo_companero.x,rumbo_companero.z),{0,1,0});
            const glm::mat4 cuerpo = glm::scale(raiz,glm::vec3(estatura_companero)) * companero.Normalizacion();
            glUniformMatrix4fv(shader.GetModelLocation(),1,GL_FALSE,glm::value_ptr(cuerpo));
            companero.Dibujar(GLuint(programa));
        }
    }

private:
    static std::vector<ClavePatrulla> LeerRuta(const std::string& archivo)
    {
        std::ifstream entrada(archivo);
        std::vector<ClavePatrulla> claves;
        for (std::string linea; std::getline(entrada,linea);)
        {
            if (linea.empty() || linea.front() == '#') continue;
            std::istringstream datos(linea);
            ClavePatrulla clave{};
            if (!(datos >> clave.segundo >> clave.punto.x >> clave.punto.y >> clave.punto.z) ||
                !std::isfinite(clave.segundo) || !std::isfinite(clave.punto.x) ||
                !std::isfinite(clave.punto.y) || !std::isfinite(clave.punto.z) ||
                (claves.empty() ? clave.segundo != 0 : clave.segundo <= claves.back().segundo))
            {
                claves.clear();
                break;
            }
            claves.push_back(clave);
        }
        if (!entrada.eof() || claves.size() < 3 ||
            glm::distance(claves.front().punto, claves.back().punto) > .001f)
        {
            fprintf(stderr,"Ruta de patrulla invalida: %s\n",archivo.c_str());
            claves.clear();
        }
        return claves;
    }

    static glm::vec3 Evaluar(const std::vector<ClavePatrulla>& ruta, float tiempo)
    {
        tiempo = std::fmod(tiempo,ruta.back().segundo);
        if (tiempo < 0) tiempo += ruta.back().segundo;
        const auto siguiente = std::upper_bound(ruta.begin()+1,ruta.end(),tiempo,
            [](float t, const ClavePatrulla& clave) { return t < clave.segundo; });
        if (siguiente == ruta.end()) return ruta.back().punto;
        const auto& anterior = *(siguiente-1);
        return glm::mix(anterior.punto,siguiente->punto,
                        (tiempo-anterior.segundo)/(siguiente->segundo-anterior.segundo));
    }

    PlanetaAzul::ModeloAnimado marine, companero;
    glm::vec3 posicion_companero{}, rumbo_companero{};
    float tiempo_companero = 0, estatura_companero = 0;
    bool companero_cargado = false, companero_activo = false;
    Model linterna;
    Shader shader;
    std::array<std::vector<ClavePatrulla>,4> rutas;
    std::array<float,4> tiempos{}, marcha{};
    std::array<glm::mat4,3> matrices;
    glm::mat4 marco_linterna{1};
    glm::vec3 rumbo{0,0,1};
    float tamano = 1;
    int escena_activa = 0;
    bool cargado = false, activo = false;
};

// El 8 vuelve a recorrer las vistas auxiliares si la escena tiene mas de una.
int SeleccionarCamara(int escena, int modo, const Window& ventana)
{
    if (ventana.TeclaPresionada(GLFW_KEY_6)) return 0;
    if (ventana.TeclaPresionada(GLFW_KEY_7)) return escena == 0 ? 1 : 7;
    if (ventana.TeclaPresionada(GLFW_KEY_9)) return 11;
    if (!ventana.TeclaPresionada(GLFW_KEY_8)) return modo;
    switch (escena)
    {
    case 0: return 7;
    case 1:
        if (modo == 2) return 10;
        if (modo == 10) return 5;
        return modo == 5 ? 9 : 2;
    case 2: return 6;
    case 3: return modo == 3 ? 4 : 3;
    default: return 0;
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
    auto* espacio = new EscenaEspacio();
    auto* azul = new EscenaPlanetaAzul();
    std::vector<Escena*> escenas = {espacio, planeta_naranja, new EscenaPlanetaMorado(), azul};
    for (Escena* escena : escenas)
    {
        ventana.setTitulo((std::string("Proyecto Final | Cargando ") + escena->Nombre()).c_str());
        escena->Inicializar();
    }
    //>>>>>>>>>>>>>>>>>>>>>> Luces y estado local del programa >>>>>>>>>>>
    DirectionalLight luz_general(1, 1, 1, 0.3f, 0.6f, 0, -1, -0.5f);
    LucesNaves luces;
    PatrullasMarine patrullas;
    patrullas.Cargar();
    bool linterna_encendida = true;
    bool luz_verde_encendida = true;
    bool luz_pista_encendida = true;

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
        patrullas.Actualizar(escena_actual,*escena,dt,espacio->MarcoEstacion());
        patrullas.ActualizarCompanero(escena_actual,*escena,azul->CentroAldea(),dt);
        if (escena_actual == 1 && ventana.TeclaPresionada(GLFW_KEY_P))
            luz_verde_encendida = !luz_verde_encendida;
        if (ventana.TeclaPresionada(GLFW_KEY_H)) linterna_encendida = !linterna_encendida;
        if (ventana.TeclaPresionada(GLFW_KEY_J) && escena_actual == 2)
            luz_pista_encendida = !luz_pista_encendida;

        //>>>>>>>>>>>>>>>>>>>>>> Camara y proyeccion >>>>>>>>>>>>>>>>>>>>>>>>>
        Camera& camara = escena->ObtenerCamara();
        camara_activa = SeleccionarCamara(escena_actual,camara_activa,ventana);

        glm::vec3 objetivo, rumbo;
        float distancia, altura;
        glm::mat4 vista;
        glm::vec3 ojo;
        const float raton_x = ventana.getXChange();
        const float raton_y = ventana.getYChange();
        if (camara_activa == 7 ? patrullas.Objetivo(objetivo,rumbo,distancia,altura)
                               : camara_activa == 11 ? patrullas.ObjetivoCompanero(objetivo,rumbo,distancia,altura)
                               : escena->ObjetivoCamara(camara_activa,objetivo,rumbo,distancia,altura))
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
        escena->PrepararLuces(ojo);
        luces.cantidad_puntuales = luces.cantidad_focos = 0;
        if (escena_actual == 1)
        {
            if (luz_verde_encendida)
            {
                const glm::vec3 p = glm::vec3(planeta_naranja->MarcoNaveCamara() * glm::vec4(0,2.4f,0,1));
                luces.puntuales[luces.cantidad_puntuales++] =
                    PointLight(.12f,1,.20f,0,4,p.x,p.y,p.z,1,.055f,.009f);
            }
            ActualizarLucesNave(planeta_naranja->ObtenerMatrizNave(), ventana, luces);
            ActualizarLucesNave(planeta_naranja->ObtenerMatrizNaveEspiral(), ventana, luces);
            luces.focos[luces.cantidad_focos++] = CrearFaroRival(planeta_naranja->MarcoFaroRival(false));
            luces.focos[luces.cantidad_focos++] = CrearFaroRival(planeta_naranja->MarcoFaroRival(true));
        }

        for (unsigned int i = 0;
             i < escena->NumLucesPuntuales() && luces.cantidad_puntuales < max_point_lights; ++i)
            luces.puntuales[luces.cantidad_puntuales++] = escena->ObtenerLucesPuntuales()[i];

        if ((escena_actual == 3 || (escena_actual == 2 && luz_pista_encendida)) &&
            luces.cantidad_puntuales < max_point_lights)
            luces.puntuales[luces.cantidad_puntuales++] = escena->LuzLocal();
        for (unsigned int i = 0; i < escena->NumFocos() && luces.cantidad_focos < max_spot_lights; ++i)
            luces.focos[luces.cantidad_focos++] = escena->ObtenerFocos()[i];
        if (linterna_encendida && patrullas.Activo() && luces.cantidad_focos < max_spot_lights)
            luces.focos[luces.cantidad_focos++] = patrullas.Luz();

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
        patrullas.Dibujar(shader,ctx,luz_escena ? *luz_escena : luz_general,luces);

        glUseProgram(0);
        ventana.swapBuffers();
    }

    //>>>>>>>>>>>>>>>>>>>>>> Liberacion de escenas >>>>>>>>>>>>>>>>>>>>>>>
    for (Escena* escena : escenas)
        delete escena;
    return 0;
}
