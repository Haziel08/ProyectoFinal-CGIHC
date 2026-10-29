#include "EscenaPlanetaNaranja.h"
#include "Model.h"
#include "Texture.h"
#include "Mesh_tn.h"
#include <vector>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include "RecorridoNaranja.h"

namespace
{
//>>>>>>>>>>>>>>>>>>>>>> Dimensiones y escalas >>>>>>>>>>>>>>>>>>>>>>>
constexpr float ESCALA_PISTA = 0.18f;
constexpr float ESCALA_NAVE = 1.0f;
constexpr float TAMANO_SUELO = 900.0f;
constexpr float ALTURA_SUELO = -3900.0f;
constexpr float REPETICIONES_UV = 15.0f;

//>>>>>>>>>>>>>>>>>>>>>> Modelos y texturas >>>>>>>>>>>>>>>>>>>>>>>>>>
Model pista;
Model nave;
Texture* fuego = nullptr;
Texture* texturaSuelo = nullptr;
MeshModel* suelo = nullptr;
MeshModel* mallaFuego = nullptr;

//>>>>>>>>>>>>>>>>>>>>>> Transformaciones >>>>>>>>>>>>>>>>>>>>>>>>>>>
glm::mat4 matrizPista(1.0f);
glm::mat4 matrizNave(1.0f);

//>>>>>>>>>>>>>>>>>>>>>> Variables de animacion >>>>>>>>>>>>>>>>>>>>>>
std::vector<float> distancias;
float avanceNave = 0.0f;
float longitudCircuito = 0.0f;
bool motor1 = true;
bool motor2 = true;
}

//>>>>>>>>>>>>>>>>>>>>>> Creacion de la malla del suelo >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CrearSuelo()
{
    // Dos triángulos, cara visible hacia +Y. UV repetidas sin estirar.
    const GLfloat mitad = TAMANO_SUELO * 0.5f;
    GLfloat vertices[] = {
        // X      Y             Z       U                 V                 NX  NY  NZ
        -mitad, ALTURA_SUELO, -mitad, 0.0f,             0.0f,              0, 1, 0,
         mitad, ALTURA_SUELO, -mitad, REPETICIONES_UV,  0.0f,              0, 1, 0,
        -mitad, ALTURA_SUELO,  mitad, 0.0f,             REPETICIONES_UV,   0, 1, 0,
         mitad, ALTURA_SUELO,  mitad, REPETICIONES_UV,  REPETICIONES_UV,   0, 1, 0
    };
    unsigned int indices[] = {0, 2, 1, 1, 2, 3};
    suelo = new MeshModel();
    suelo->CreateMeshModel(vertices, indices, 32, 6);
}

//>>>>>>>>>>>>>>>>>>>>>> Inicializacion y carga de recursos >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::Inicializar()
{
    CargarSkybox("assets/planeta_naranja/skybox/");
    // Ojos a altura de una persona sobre el suelo, sin quedar dentro de la malla.
    camara = Camera(glm::vec3(110.0f, ALTURA_SUELO + 1.7f, 55.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f), -120.0f, 0.0f, 35.0f, 0.3f);

    //>>>>>>>>>>>>>>>>>>>>>> Carga de texturas >>>>>>>>>>>>>>>>>>>>>>>>>>
    CrearSuelo();
    texturaSuelo = new Texture("assets/planeta_naranja/texturas/suelo_naranja.jpg");
    if (!texturaSuelo->LoadTexture())
        printf("No se pudo cargar el suelo naranja.\n");

    //>>>>>>>>>>>>>>>>>>>>>> Carga de modelos >>>>>>>>>>>>>>>>>>>>>>>>>>>
    pista.LoadModel("assets/planeta_naranja/modelos/pista_3_carriles.obj");
    nave.LoadModel("assets/planeta_naranja/modelos/nave_espacial.obj");
    fuego = new Texture("assets/planeta_naranja/texturas/fuego.png");
    fuego->LoadTextureA();
    CrearFuego();

    // La planta de 2700 x 3300 queda centrada y ocupa 486 x 594 del suelo de 900.
    matrizPista = glm::translate(glm::mat4(1.0f),
        glm::vec3(390.0f * ESCALA_PISTA, ALTURA_SUELO + 0.5f, -90.0f * ESCALA_PISTA));
    matrizPista = glm::scale(matrizPista, glm::vec3(ESCALA_PISTA));
    distancias.clear();
    distancias.push_back(0.0f);
    for (int i = 1; i < RecorridoNaranja::CANTIDAD; ++i)
        distancias.push_back(distancias.back() + glm::length(
            RecorridoNaranja::PUNTOS[i] - RecorridoNaranja::PUNTOS[i - 1]));
    longitudCircuito = distancias.back();
    avanceNave = 0.0f;
    ActualizarNave(0.0f);
}

//>>>>>>>>>>>>>>>>>>>>>> Actualizacion de la escena >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::Actualizar(float dt, Window& ventana)
{
    motor1 = ventana.getMotor1Encendido();
    motor2 = ventana.getMotor2Encendido();
    if (ventana.getVistaGeneralSolicitada())
        camara = Camera(glm::vec3(0.0f, ALTURA_SUELO + 390.0f, 430.0f),
            glm::vec3(0, 1, 0), -90.0f, -40.0f, 80.0f, 0.3f);
    ActualizarNave(dt);
}

//>>>>>>>>>>>>>>>>>>>>>> Puesta en escena >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::Dibujar(const ContextoRender& ctx)
{
    if (!suelo || !texturaSuelo) return;
    const glm::mat4 matrizSuelo(1.0f);
    glUniformMatrix4fv(ctx.uModel, 1, GL_FALSE, glm::value_ptr(matrizSuelo));
    glUniform3f(ctx.uColor, 1.0f, 1.0f, 1.0f);
    glUniform1f(ctx.uSpecularIntensity, 0.08f);
    glUniform1f(ctx.uShininess, 8.0f);
    texturaSuelo->UseTexture();
    suelo->RenderMeshModel();

    //>>>>>>>>>>>>>>>>>>>>>> Pista completa >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    glUniformMatrix4fv(ctx.uModel, 1, GL_FALSE, glm::value_ptr(matrizPista));
    glUniform1f(ctx.uSpecularIntensity, 0.25f);
    glUniform1f(ctx.uShininess, 32.0f);
    pista.RenderModel();

    //>>>>>>>>>>>>>>>>>>>>>> Nave espacial >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    glUniformMatrix4fv(ctx.uModel, 1, GL_FALSE, glm::value_ptr(matrizNave));
    glUniform1f(ctx.uSpecularIntensity, 0.6f);
    glUniform1f(ctx.uShininess, 64.0f);
    nave.RenderModel();

    // Planos cruzados con transparencia y mezcla aditiva.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glUniform3f(ctx.uColor, 2.0f, 2.0f, 2.0f);
    glUniform1f(ctx.uSpecularIntensity, 0.0f);
    fuego->UseTexture();
    if (motor1) DibujarFuego(ctx, 4.0f);
    if (motor2) DibujarFuego(ctx, -4.0f);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUniform3f(ctx.uColor, 1.0f, 1.0f, 1.0f);
}

//>>>>>>>>>>>>>>>>>>>>>> Trayectoria por segmentos >>>>>>>>>>>>>>>>>>>>>>
glm::vec3 EscenaPlanetaNaranja::PuntoPista(float distancia) const
{
    float s = std::fmod(distancia, longitudCircuito);
    if (s < 0.0f) s += longitudCircuito;
    auto siguiente = std::upper_bound(distancias.begin(), distancias.end(), s);
    size_t i = static_cast<size_t>(siguiente - distancias.begin() - 1);
    float u = (s - distancias[i]) / (distancias[i + 1] - distancias[i]);
    return glm::mix(RecorridoNaranja::PUNTOS[i], RecorridoNaranja::PUNTOS[i + 1], u);
}

//>>>>>>>>>>>>>>>>>>>>>> Actualizacion de la escena >>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>> Animacion y orientacion de la nave >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::ActualizarNave(float dt)
{
    // Distancia real por segundo; conserva el sobrante al pasar de segmento.
    avanceNave = std::fmod(avanceNave + (12.0f / ESCALA_PISTA) * dt, longitudCircuito);
    const float semilargo = 13.0f * ESCALA_NAVE;
    glm::vec3 centro = PuntoPista(avanceNave);
    glm::vec3 adelante = glm::normalize(PuntoPista(avanceNave + semilargo)
        - PuntoPista(avanceNave - semilargo));
    glm::vec3 derecha = glm::normalize(glm::cross(adelante, glm::vec3(0, 1, 0)));
    glm::vec3 arriba = glm::normalize(glm::cross(derecha, adelante));
    // Anticipar la altura de la rampa debajo de toda la longitud del casco.
    float holgura = 0.0f;
    for (int k = -8; k <= 8; ++k)
        holgura = std::max(holgura, glm::dot(
            PuntoPista(avanceNave + semilargo * k / 8.0f) - centro, arriba));
    glm::vec3 posicion = centro + arriba * (4.0f + holgura);
    // El OBJ tiene los motores en +X: la nariz mira a -X.
    glm::mat4 orientacion(glm::vec4(-adelante, 0), glm::vec4(arriba, 0),
        glm::vec4(-derecha, 0), glm::vec4(posicion, 1));
    matrizNave = glm::scale(matrizPista * orientacion, glm::vec3(ESCALA_NAVE));
}

//>>>>>>>>>>>>>>>>>>>>>> Creacion de la malla del fuego >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CrearFuego()
{
    // X, Y, Z, U, V, NX, NY, NZ; V=0 en la salida del motor.
    GLfloat v[] = {
        -0.5f,0,0, 1,0, 0,1,0,  0.5f,0,0, 0,0, 0,1,0,
         0.5f,0,1, 0,1, 0,1,0, -0.5f,0,1, 1,1, 0,1,0,
         0,-0.5f,0, 1,0, 1,0,0, 0,0.5f,0, 0,0, 1,0,0,
         0,0.5f,1, 0,1, 1,0,0, 0,-0.5f,1, 1,1, 1,0,0
    };
    unsigned int indices[] = {0,1,2, 0,2,3, 4,5,6, 4,6,7};
    mallaFuego = new MeshModel();
    mallaFuego->CreateMeshModel(v, indices, 64, 12);
}

//>>>>>>>>>>>>>>>>>>>>>> Puesta en escena >>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>> Puesta en escena del fuego >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::DibujarFuego(const ContextoRender& ctx, float lado)
{
    glm::mat4 m = glm::translate(matrizNave, glm::vec3(13, 0, lado));
    // El +Z del plano se orienta hacia +X local, detrás de los motores.
    m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0, 1, 0));
    m = glm::scale(m, glm::vec3(4, 4, 5.4f));
    glUniformMatrix4fv(ctx.uModel, 1, GL_FALSE, glm::value_ptr(m));
    mallaFuego->RenderMeshModel();
}

//>>>>>>>>>>>>>>>>>>>>>> Liberacion de recursos >>>>>>>>>>>>>>>>>>>>>>
EscenaPlanetaNaranja::~EscenaPlanetaNaranja()
{
    pista.ClearModel();
    nave.ClearModel();
    delete fuego;
    delete mallaFuego;
    delete texturaSuelo;
    delete suelo;
}

//>>>>>>>>>>>>>>>>>>>>>> Transformacion de la nave >>>>>>>>>>>>>>>>>>>
glm::mat4 EscenaPlanetaNaranja::ObtenerMatrizNave() const
{
    return matrizNave;
}
