//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y cabeceras >>>>>>>>>>>>>>>>>>>>>
#include "EscenaPlanetaNaranja.h"
#include "RecorridoNaranja.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Solo configuracion inmutable compartida; el estado pertenece a la escena.
namespace
{
const glm::vec3 posicion_torre_blanca(-155, 0, -85);
const glm::vec3 posicion_torre_control(-100, 0, 90);
const glm::vec3 pivotes_rotores[2] = {{-0.099f, 0.495f, -0.407f}, {-0.099f, 0.495f, 0.407f}};
//>>>>>>>>>>>>>>>>>>>>>> Modelos decorativos del planeta >>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>> Llanura suave y explanadas de los edificios >>>>>>>>>>>>>>>>>>
float AlturaSuelo(float x, float z)
{
    float t = glm::clamp(std::max(std::abs(x) - 310.0f, std::abs(z) - 360.0f) / 170.0f, 0.0f, 1.0f);
    float altura = t * t * (3.0f - 2.0f * t) *
                   (2.8f + 1.8f * std::sin(x * .012f) * std::cos(z * .010f) +
                    1.1f * std::sin((x + z) * .018f) + .7f * std::cos(x * .027f - z * .021f));
    const glm::vec3 explanadas[] = {
        {395, 425, 65}, {400, -380, 55}, {-440, -380, 55}, {-410, 280, 40}, {340, 140, 40}};
    for (const auto& explanada : explanadas)
    {
        float u = glm::clamp(
            (glm::length(glm::vec2(x - explanada.x, z - explanada.y)) - explanada.z) / 20.0f, 0.0f, 1.0f);
        altura *= u * u * (3.0f - 2.0f * u);
    }
    return altura;
}
enum TipoDecoracion
{
    casa_uno,
    casa_dos,
    arco_casas,
    tanque_pequeno,
    deposito_horizontal,
    tanque_grande,
    tanques_cuatro,
    tuberias,
    radiotelescopio,
    arbol_uno,
    arbol_dos,
    arbol_tres,
    arbol_cuatro,
    arbol_portal,
    domo_cristal,
    portal_azul,
    bordes_domo
};
const char* nombres_decoracion[] = {
    "casa_tatooine_1", "casa_tatooine_2", "arco_tatooine", "tanque_pequeno",  "deposito_horizontal",
    "tanque_grande",   "tanques_cuatro",  "tuberias",      "radiotelescopio", "arbol_seco_1",
    "arbol_seco_2",    "arbol_seco_3",    "arbol_seco_4",  "arbol_portal",    "domo_cristal",
    "portal_azul",     "bordes_domo"};
//>>>>>>>>>>>>>>>>>>>>>> Trafico por carriles >>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* nombres_rivales[] = {"nave_17", "nave_18", "nave_22", "nave_24",
                                 "nave_25", "nave_28", "nave_33", "nave_seamoth"};
struct ConfiguracionRival
{
    int modelo;
    bool espiral;
    float carril, fase, velocidad, escala;
};
const float giros_rivales[] = {0, 0, 0, 0, -90, -90, 0, 0};
const ConfiguracionRival configuraciones_rivales[] = {
    {7, true, -16.500f, 0.000f, 0.900f, 1.000f},       {0, true, -16.500f, 1.0f / 3.0f, 0.900f, 0.920f},
    {2, true, -16.500f, 2.0f / 3.0f, 0.900f, 0.920f},  {1, true, -5.500f, 1.0f / 3.0f, 1.000f, 0.920f},
    {3, true, -5.500f, 2.0f / 3.0f, 1.000f, 0.920f},   {4, true, 5.500f, 0.000f, 1.100f, 0.920f},
    {5, true, 5.500f, 1.0f / 3.0f, 1.100f, 0.920f},    {6, true, 5.500f, 2.0f / 3.0f, 1.100f, 0.920f},
    {2, true, 16.500f, 0.000f, 1.180f, 0.920f},        {3, true, 16.500f, 1.0f / 3.0f, 1.180f, 0.920f},
    {0, true, 16.500f, 2.0f / 3.0f, 1.180f, 0.920f},   {7, false, -40.000f, 0.000f, 0.900f, 0.650f},
    {0, false, -40.000f, 1.0f / 3.0f, 0.900f, 0.620f}, {2, false, -40.000f, 2.0f / 3.0f, 0.900f, 0.620f},
    {1, false, 0.000f, 1.0f / 3.0f, 1.000f, 0.620f},   {4, false, 0.000f, 2.0f / 3.0f, 1.000f, 0.620f},
    {3, false, 40.000f, 0.000f, 1.100f, 0.620f},       {5, false, 40.000f, 1.0f / 3.0f, 1.100f, 0.620f},
    {6, false, 40.000f, 2.0f / 3.0f, 1.100f, 0.620f}};
} // namespace

//>>>>>>>>>>>>>>>>>>>>>> Inicializacion: orden de los recursos >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::Inicializar()
{
    CargarSkybox("assets/planeta_naranja/texturas/", ".jpg", "cielo_");
    cielo_dia = skybox;
    cielo_noche = Skybox({"assets/planeta_naranja/texturas/cielo_noche_derecha.jpg",
                          "assets/planeta_naranja/texturas/cielo_noche_izquierda.jpg",
                          "assets/planeta_naranja/texturas/cielo_abajo.jpg",
                          "assets/planeta_naranja/texturas/cielo_noche_arriba.jpg",
                          "assets/planeta_naranja/texturas/cielo_noche_atras.jpg",
                          "assets/planeta_naranja/texturas/cielo_noche_frente.jpg"});
    camara = Camera(glm::vec3(110, altura_suelo + 1.7f, 55), glm::vec3(0, 1, 0), -120, 0, 35, 0.3f);
    CargarModelos();
    CargarRocky();
    cohete_auxiliar.Cargar();
    CargarTexturas();
    CrearFuego();
    CrearMallasAereas();
    CrearRecorrido();
    CrearPaisajePeriferico();
    CrearDecoracion();
    CrearFaros();
    CrearIluminacion();
    CrearHalos();
    ActualizarIluminacion(0.0f);

    // Estado inicial de ambas naves, asteroides, rotores y emisores.
    avance_nave = tiempo_espiral = 0.0f;
    tiempo_carreras = 0.0;
    ActualizarNaves(0.0f);
    tiempo_aereo = angulo_aspas = emision_humo_aereo = 0.0f;
    siguiente_humo_aereo = 0;
    for (auto& particula_humo : humo_aereo)
        particula_humo.edad = vida_humo_aereo;
    ActualizarObjetosAereos(0.0f);
}

//>>>>>>>>>>>>>>>>>>>>>> Carga de modelos >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CargarModelos()
{
    for (int i = 0; i < 17; ++i)
        decorativos[i].LoadModel(
            std::string("assets/planeta_naranja/modelos/") + nombres_decoracion[i] + ".obj", false, false);
    for (int i = 0; i < 8; ++i)
        rivales[i].LoadModel(std::string("assets/planeta_naranja/modelos/") + nombres_rivales[i] + ".obj",
                             false, false);
    suelo.LoadModel("assets/planeta_naranja/modelos/suelo.obj", false, false);
    pista.LoadModel("assets/planeta_naranja/modelos/pista_inferior.obj");
    pista_espiral.LoadModel("assets/planeta_naranja/modelos/pista_espiral.obj", false, false);
    nave.LoadModel("assets/planeta_naranja/modelos/nave_espacial.obj");
    poste_luz.LoadModel("assets/planeta_naranja/modelos/faro.obj", false, false);
    soporte_faro_lateral.LoadModel("assets/planeta_naranja/modelos/apoyo_faro.obj", false, false);
    secciones_pista = LeerSeccionesPista("assets/planeta_naranja/modelos/pista_inferior.obj");
    secciones_espiral = LeerSeccionesPista("assets/planeta_naranja/modelos/pista_espiral.obj");
    soportes_y_senalizacion.LoadModel("assets/planeta_naranja/modelos/soportes_y_senalizacion.obj", false,
                                      false);
    torre_comunicaciones.LoadModel("assets/planeta_naranja/modelos/torre_comunicaciones.obj", false, false);
    torre_control.LoadModel("assets/planeta_naranja/modelos/torre_control.obj", false, false);
    for (int i = 0; i < 3; ++i)
        montanas[i].LoadModel("assets/planeta_naranja/modelos/montana_0" + std::to_string(i + 1) + ".obj",
                              false, false);
    roca_periferica.LoadModel("assets/planeta_naranja/modelos/roca_periferica.obj", false, false);
    for (int i = 0; i < 3; ++i)
        asteroides[i].LoadModel(
            "assets/planeta_naranja/modelos/asteroide_0" + std::to_string(i + 1) + ".obj", false, false);
    dron_explorador.LoadModel("assets/planeta_naranja/modelos/dron.obj", false, false);
}

//>>>>>>>>>>>>>>>>>>>>>> Carga de texturas >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CargarTexturas()
{
    fuego = new Texture("assets/planeta_naranja/texturas/fuego_motores.png");
    fuego->LoadTextureA();
    textura_fuego_asteroide = new Texture("assets/planeta_naranja/texturas/fuego_asteroides.png");
    textura_fuego_asteroide->LoadTextureA();
    textura_humo = new Texture("assets/planeta_naranja/texturas/humo.png");
    textura_humo->LoadTextureA();
    textura_aspas = new Texture("assets/planeta_naranja/texturas/placas_metalicas.png");
    textura_aspas->LoadTextureA();
}

//>>>>>>>>>>>>>>>>>>>>>> Malla del fuego de los motores >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CrearFuego()
{
    // X, Y, Z, U, V, NX, NY, NZ; V=0 en la salida del motor.
    GLfloat v[] = {-0.5f, 0,     0, 1, 0, 0, 1, 0, 0.5f,  0,     0, 0, 0, 0, 1, 0,
                   0.5f,  0,     1, 0, 1, 0, 1, 0, -0.5f, 0,     1, 1, 1, 0, 1, 0,
                   0,     -0.5f, 0, 1, 0, 1, 0, 0, 0,     0.5f,  0, 0, 0, 1, 0, 0,
                   0,     0.5f,  1, 0, 1, 1, 0, 0, 0,     -0.5f, 1, 1, 1, 1, 0, 0};
    unsigned int indices[] = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
    malla_fuego = new MeshModel();
    malla_fuego->CreateMeshModel(v, indices, 64, 12);
}

//>>>>>>>>>>>>>>>>>>>>>> Mallas de estelas, humo y aspas >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CrearMallasAereas()
{
    unsigned int indices_planos[] = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
    GLfloat estela[] = {0,  -.5f, 0,    0, 0, 0, 0, 1, 0,  .5f,  0,    1, 0, 0, 0, 1,
                        -1, .5f,  0,    1, 1, 0, 0, 1, -1, -.5f, 0,    0, 1, 0, 0, 1,
                        0,  0,    -.5f, 0, 0, 0, 1, 0, 0,  0,    .5f,  1, 0, 0, 1, 0,
                        -1, 0,    .5f,  1, 1, 0, 1, 0, -1, 0,    -.5f, 0, 1, 0, 1, 0};
    malla_estela_asteroide = new MeshModel();
    malla_estela_asteroide->CreateMeshModel(estela, indices_planos, 64, 12);
    // Misma malla XY y coordenadas UV usadas en preparaHumo() de la practica 10.
    GLfloat particula[] = {-.5f, -.5f, 0, 0, 0, 0, 0, 1, .5f,  -.5f, 0, 1, 0, 0, 0, 1,
                           .5f,  .5f,  0, 1, 1, 0, 0, 1, -.5f, .5f,  0, 0, 1, 0, 0, 1};
    malla_humo_aereo = new MeshModel();
    malla_humo_aereo->CreateMeshModel(particula, indices_planos, 32, 6);

    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;
    auto cara = [&](const std::array<glm::vec3, 4>& puntos) {
        glm::vec3 normal = glm::normalize(glm::cross(puntos[1] - puntos[0], puntos[2] - puntos[0]));
        const glm::vec2 uv[] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        unsigned int base = static_cast<unsigned int>(vertices.size() / 8);
        for (int i = 0; i < 4; ++i)
            vertices.insert(vertices.end(), {puntos[i].x, puntos[i].y, puntos[i].z, uv[i].x, uv[i].y,
                                             normal.x, normal.y, normal.z});
        for (unsigned int i : {0u, 1u, 2u, 0u, 2u, 3u})
            indices.push_back(base + i);
    };
    for (int pala = 0; pala < 4; ++pala)
    {
        glm::mat4 giro = glm::rotate(glm::mat4(1), pala * glm::half_pi<float>(), glm::vec3(0, 1, 0));
        std::array<glm::vec3, 4> arriba = {
            {{.024f, .008f, -.018f}, {.026f, .004f, .018f}, {.165f, .006f, .009f}, {.158f, .012f, -.010f}}};
        std::array<glm::vec3, 4> abajo;
        for (int i = 0; i < 4; ++i)
        {
            arriba[i] = glm::vec3(giro * glm::vec4(arriba[i], 1));
            abajo[i] = arriba[i] - glm::vec3(0, .012f, 0);
        }
        cara(arriba);
        cara({abajo[3], abajo[2], abajo[1], abajo[0]});
        for (int i = 0; i < 4; ++i)
        {
            int j = (i + 1) % 4;
            cara({arriba[i], abajo[i], abajo[j], arriba[j]});
        }
    }
    malla_aspas_dron = new MeshModel();
    malla_aspas_dron->CreateMeshModel(vertices.data(), indices.data(),
                                      static_cast<unsigned int>(vertices.size()),
                                      static_cast<unsigned int>(indices.size()));
}

//>>>>>>>>>>>>>>>>>>>>>> Distribucion: recorrido y matriz de la pista >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CrearRecorrido()
{
    // La planta de 2700 x 3300 queda centrada y ocupa 486 x 594 del suelo de 1350.
    matriz_pista = glm::translate(
        glm::mat4(1.0f), glm::vec3(390.0f * escala_pista, altura_suelo + 0.5f, -90.0f * escala_pista));
    matriz_pista = glm::scale(matriz_pista, glm::vec3(escala_pista));
    distancias.clear();
    distancias.push_back(0.0f);
    for (int i = 1; i < RecorridoNaranja::cantidad; ++i)
        distancias.push_back(distancias.back() +
                             glm::length(RecorridoNaranja::puntos[i] - RecorridoNaranja::puntos[i - 1]));
    longitud_circuito = distancias.back();
}

//>>>>>>>>>>>>>>>>>>>>>> Distribucion: montanas y rocas >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CrearPaisajePeriferico()
{
    montanas_perifericas.clear();
    rocas_perifericas.clear();
    auto agregar = [this](glm::vec3 posicion, float largo, float alto, float fondo, float giro, int indice,
                          bool incluir_roca) {
        glm::mat4 matriz = glm::translate(glm::mat4(1), posicion + glm::vec3(0, altura_suelo - 14.0f, 0));
        matriz = glm::rotate(matriz, glm::radians(giro), glm::vec3(0, 1, 0));
        montanas_perifericas.push_back({glm::scale(matriz, glm::vec3(largo, alto, fondo)), indice % 3});
        if (incluir_roca)
        {
            float variacion = float((indice * 31 + 17) % 101) / 100.0f;
            glm::vec3 tangente = glm::vec3(std::cos(glm::radians(giro)), 0, -std::sin(glm::radians(giro)));
            glm::vec3 normal = glm::normalize(glm::vec3(posicion.x, 0, posicion.z));
            glm::vec3 pie =
                posicion - normal * (fondo * 0.5f - 35.0f) + tangente * ((variacion - 0.5f) * 110.0f);
            bool gigante = indice % 3 == 1 || indice == 0;
            glm::mat4 roca = glm::translate(glm::mat4(1),
                                            pie + glm::vec3(0, altura_suelo - (gigante ? 14.0f : 8.0f), 0));
            roca = glm::rotate(roca, glm::radians(giro + variacion * 45.0f), glm::vec3(0, 1, 0));
            rocas_perifericas.push_back(
                glm::scale(roca, gigante ? glm::vec3(190.0f + variacion * 45.0f, 135.0f + variacion * 40.0f,
                                                     260.0f + variacion * 50.0f)
                                         : glm::vec3(110.0f + variacion * 40.0f, 75.0f + variacion * 35.0f,
                                                     155.0f + variacion * 55.0f)));
        }
    };
    for (int lado = 0; lado < 4; ++lado)
        for (int i = 0; i < 3; ++i)
        {
            int indice = lado * 3 + i;
            float v = float((indice * 37 + 19) % 101) / 100.0f;
            float posicion = -550.0f + 550.0f * i;
            glm::vec3 centro = lado == 0   ? glm::vec3(-845, 0, posicion)
                               : lado == 1 ? glm::vec3(845, 0, posicion)
                               : lado == 2 ? glm::vec3(posicion, 0, -845)
                                           : glm::vec3(posicion, 0, 845);
            float giro = lado < 2 ? 90.0f : 0.0f;
            bool central = i == 1;
            if (central)
                centro *= 760.0f / 845.0f;
            agregar(centro, (820.0f + 80.0f * v) * (central ? 1.35f : 1.0f), 3.0f * (105.0f + 95.0f * v),
                    central ? 480.0f : 300.0f, giro + std::sin(float(indice)), indice, true);
        }
    // Faldas bajas en primer plano, solapadas con las dos montanas de cada esquina.
    for (int esquina = 0; esquina < 4; ++esquina)
    {
        float sx = (esquina & 1) ? 1.0f : -1.0f;
        float sz = (esquina & 2) ? 1.0f : -1.0f;
        float giro = glm::degrees(std::atan2(sx, sz));
        agregar(glm::vec3(sx * 650.0f, 0, sz * 650.0f), 660.0f, 90.0f + esquina * 10.0f, 340.0f, giro,
                12 + esquina, false);
    }
}

//>>>>>>>>>>>>>>>>>>>>>> Pueblo, bosque y equipamiento del planeta >>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CrearDecoracion()
{
    struct Ubicacion
    {
        TipoDecoracion tipo;
        glm::vec3 posicion;
        float escala, giro;
    };
    const Ubicacion distribucion[] = {
        {tanques_cuatro, {340.000f, 0.000f, 140.000f}, 25.000f, 25.000f},
        {tanque_grande, {-410.000f, 0.000f, 280.000f}, 31.000f, 20.000f},
        {radiotelescopio, {400.000f, 0.000f, -380.000f}, 58.000f, -20.000f},
        {domo_cristal, {-225.000f, 0.000f, -75.000f}, 38.000f, 0.000f},
        {domo_cristal, {395.000f, 0.000f, 425.000f}, 44.000f, 0.000f},
        {radiotelescopio, {-350.000f, -0.252f, -350.000f}, 52.000f, 35.000f},
        {tanque_pequeno, {-56.000f, -0.120f, 113.000f}, 17.000f, 20.000f},
        {tanque_pequeno, {-119.000f, -0.120f, -85.000f}, 17.000f, 0.000f},
        {deposito_horizontal, {-140.000f, -0.120f, -140.000f}, 13.500f, 0.000f},
        {tuberias, {-100.000f, -0.120f, 46.000f}, 9.000f, 90.000f},
        {tuberias, {-193.000f, -0.120f, -126.000f}, 9.000f, 0.000f},
        {arbol_portal, {262.000f, 0.000f, 240.000f}, 24.000f, 10.000f},
        {arbol_uno, {100.000f, -2.331f, 20.000f}, 25.000f, 37.000f},
        {arbol_tres, {-190.000f, -2.004f, 135.000f}, 22.000f, 112.000f},
        {arbol_cuatro, {165.000f, -2.240f, -120.000f}, 28.000f, 215.000f},
        {arbol_uno, {361.778f, -3.361f, 490.940f}, 29.106f, 340.552f},
        {arbol_dos, {432.515f, -2.393f, 274.114f}, 22.348f, 28.398f},
        {arbol_tres, {328.376f, -2.306f, 465.765f}, 21.342f, 214.109f},
        {arbol_cuatro, {462.917f, -2.248f, 301.133f}, 27.616f, 291.339f},
        {arbol_uno, {259.619f, -2.667f, 300.924f}, 29.587f, 12.514f},
        {arbol_dos, {307.272f, -2.865f, 517.879f}, 27.569f, 72.988f},
        {arbol_tres, {532.495f, -2.723f, 355.588f}, 30.464f, 96.085f},
        {arbol_cuatro, {385.647f, -1.475f, 500.423f}, 23.748f, 88.006f},
        {arbol_uno, {526.532f, -2.657f, 234.280f}, 28.860f, 289.198f},
        {arbol_dos, {485.775f, -2.817f, 270.963f}, 26.313f, 224.067f},
        {arbol_uno, {49.906f, -2.135f, -115.936f}, 22.329f, 203.538f},
        {arbol_dos, {169.553f, -2.862f, -161.672f}, 29.640f, 66.581f},
        {arbol_tres, {119.522f, -2.260f, 52.461f}, 25.740f, 193.588f},
        {arbol_cuatro, {-8.467f, -1.877f, 180.962f}, 22.160f, 48.513f},
        {arbol_uno, {174.030f, -2.456f, -136.968f}, 26.702f, 124.069f},
        {arbol_dos, {190.098f, -2.659f, 144.905f}, 27.089f, 199.009f},
        {arbol_tres, {172.525f, -2.487f, 78.006f}, 29.061f, 67.136f},
        {arbol_uno, {-268.568f, -2.358f, -506.894f}, 25.546f, 38.800f},
        {arbol_dos, {-463.210f, -2.753f, 77.931f}, 29.873f, 110.002f},
        {arbol_tres, {-329.286f, -2.036f, 369.526f}, 22.205f, 218.464f},
        {arbol_cuatro, {-249.427f, -2.103f, 502.386f}, 26.367f, 341.507f},
        {arbol_uno, {-446.604f, -2.633f, -50.653f}, 29.208f, 345.768f},
        {arbol_dos, {95.437f, -2.598f, -398.945f}, 27.379f, 280.151f},
        {arbol_tres, {-438.992f, -2.379f, 576.842f}, 26.185f, 247.322f},
        {arbol_cuatro, {-548.206f, -2.310f, -129.562f}, 30.323f, 107.597f},
        {arbol_uno, {587.957f, -2.609f, 345.668f}, 27.066f, 251.015f},
        {arbol_dos, {228.610f, 7.251f, -576.620f}, 25.268f, 291.533f},
        {arbol_tres, {574.772f, -2.009f, 372.096f}, 20.394f, 259.360f},
        {arbol_cuatro, {304.720f, -2.108f, -503.231f}, 24.289f, 181.776f},
        {arbol_uno, {-864.361f, 67.529f, -793.452f}, 27.226f, 41.348f},
        {arbol_dos, {254.042f, 141.142f, -687.041f}, 40.884f, 64.567f},
        {arbol_tres, {-655.240f, 41.479f, -634.708f}, 38.604f, 256.081f},
        {arbol_cuatro, {441.360f, 50.000f, -926.786f}, 27.651f, 136.476f},
        {arbol_uno, {399.976f, 98.086f, -878.060f}, 32.428f, 250.440f},
        {arbol_dos, {-716.229f, 21.526f, -629.117f}, 33.036f, 56.101f},
        {arbol_tres, {93.602f, 165.143f, -652.854f}, 35.676f, 219.688f},
        {arbol_cuatro, {-180.612f, 24.612f, -922.381f}, 41.220f, 216.506f},
        {arbol_uno, {-108.364f, 35.962f, -924.616f}, 35.472f, 124.149f},
        {arbol_dos, {706.663f, 33.644f, 702.556f}, 26.503f, 167.225f},
        {arbol_tres, {861.251f, 33.073f, 884.625f}, 25.499f, 258.699f},
        {arbol_cuatro, {479.515f, 139.215f, 807.418f}, 26.062f, 166.420f},
        {arbol_uno, {-127.254f, 82.008f, 912.038f}, 41.092f, 46.765f},
        {arbol_tres, {65.595f, 500.791f, 755.130f}, 28.242f, 259.994f},
        {arbol_cuatro, {-492.819f, 147.277f, 922.308f}, 33.200f, 254.369f},
        {arbol_uno, {7.328f, 280.875f, 820.608f}, 39.111f, 99.871f},
        {arbol_dos, {-236.517f, 168.320f, 788.406f}, 37.566f, 309.514f},
        {arbol_tres, {-789.424f, 145.286f, -324.000f}, 28.111f, 254.047f},
        {arbol_cuatro, {-910.844f, 53.880f, 130.995f}, 35.368f, 271.993f},
        {arbol_uno, {-698.209f, 26.951f, 580.385f}, 25.157f, 324.612f},
        {arbol_dos, {-759.026f, 37.221f, -746.841f}, 28.569f, 73.557f},
        {arbol_cuatro, {-923.227f, 18.256f, -727.611f}, 31.189f, 83.251f},
        {arbol_uno, {-862.275f, 109.476f, -142.715f}, 36.954f, 292.910f},
        {arbol_dos, {-908.033f, 80.669f, 754.532f}, 28.277f, 116.531f},
        {arbol_tres, {-672.185f, 85.320f, 225.916f}, 38.041f, 5.859f},
        {arbol_cuatro, {639.785f, 54.814f, -681.918f}, 30.264f, 213.882f},
        {arbol_uno, {693.177f, 21.720f, 319.870f}, 38.449f, 236.747f},
        {arbol_dos, {776.967f, 66.660f, 369.454f}, 36.175f, 259.305f},
        {arbol_tres, {624.680f, 81.105f, 909.403f}, 24.741f, 178.961f},
        {arbol_cuatro, {745.394f, 120.608f, 286.395f}, 38.772f, 283.262f},
        {arbol_uno, {876.651f, 32.368f, 849.069f}, 25.650f, 31.099f},
        {arbol_dos, {877.532f, 208.527f, -453.678f}, 34.614f, 202.822f},
        {arbol_tres, {846.973f, 134.035f, -704.443f}, 30.111f, 172.310f},
        {arbol_cuatro, {762.923f, 28.948f, 699.675f}, 31.752f, 344.391f},
        {arbol_uno, {-832.221f, 80.091f, -788.452f}, 31.519f, 1.867f},
        {arbol_dos, {-835.505f, 96.285f, -777.954f}, 26.480f, 220.522f},
        {arbol_tres, {-822.823f, 94.801f, -762.791f}, 38.129f, 70.397f},
        {arbol_cuatro, {-874.624f, 42.549f, -829.831f}, 27.148f, 297.204f},
        {arbol_uno, {-876.741f, 23.082f, -859.379f}, 30.876f, 81.266f},
        {arbol_dos, {-827.219f, 95.331f, -779.237f}, 35.345f, 104.741f},
        {arbol_tres, {-835.732f, 100.185f, -767.663f}, 36.731f, 122.329f},
        {arbol_uno, {437.042f, 117.875f, -866.895f}, 41.121f, 226.681f},
        {arbol_dos, {433.560f, 68.564f, -905.518f}, 31.983f, 201.551f},
        {arbol_tres, {422.483f, 31.733f, -924.160f}, 35.828f, 185.247f},
        {arbol_cuatro, {451.070f, 40.616f, -934.980f}, 42.972f, 94.490f},
        {arbol_uno, {399.713f, 87.973f, -891.449f}, 37.563f, 267.334f},
        {arbol_dos, {417.968f, 32.046f, -939.426f}, 40.349f, 49.070f},
        {arbol_tres, {445.853f, 111.278f, -888.768f}, 29.798f, 91.648f},
        {arbol_uno, {84.787f, 164.088f, -646.626f}, 41.402f, 321.399f},
        {arbol_dos, {123.330f, 179.727f, -655.213f}, 30.890f, 233.678f},
        {arbol_tres, {94.775f, 176.719f, -659.494f}, 42.315f, 211.222f},
        {arbol_cuatro, {58.381f, 121.422f, -629.016f}, 35.473f, 176.965f},
        {arbol_uno, {111.351f, 297.743f, -697.115f}, 36.383f, 37.950f},
        {arbol_dos, {115.853f, 182.854f, -668.314f}, 29.972f, 16.160f},
        {arbol_tres, {109.195f, 167.604f, -654.788f}, 37.261f, 186.452f},
        {arbol_uno, {706.454f, 35.748f, 713.188f}, 42.118f, 0.324f},
        {arbol_dos, {705.771f, 30.520f, 683.928f}, 42.289f, 166.124f},
        {arbol_tres, {683.395f, 49.445f, 699.993f}, 38.650f, 176.193f},
        {arbol_cuatro, {666.846f, 59.083f, 709.611f}, 39.820f, 298.500f},
        {arbol_uno, {692.550f, 28.081f, 738.932f}, 29.887f, 256.441f},
        {arbol_dos, {657.101f, 72.510f, 655.136f}, 26.377f, 254.314f},
        {arbol_tres, {688.563f, 43.768f, 705.687f}, 37.454f, 231.430f},
        {arbol_uno, {-177.127f, 29.333f, 933.847f}, 39.061f, 154.980f},
        {arbol_dos, {-195.853f, 66.676f, 922.971f}, 29.469f, 180.679f},
        {arbol_tres, {-155.701f, 91.099f, 896.527f}, 32.496f, 234.718f},
        {arbol_cuatro, {-134.870f, 55.245f, 930.287f}, 37.499f, 124.613f},
        {arbol_uno, {-120.640f, 119.634f, 881.684f}, 29.731f, 142.077f},
        {arbol_dos, {-174.643f, 76.844f, 896.254f}, 34.205f, 54.048f},
        {arbol_tres, {-109.810f, 63.644f, 937.384f}, 42.331f, 89.501f},
        {arbol_uno, {-3.309f, 209.688f, 850.499f}, 37.527f, 356.552f},
        {arbol_dos, {40.186f, 320.695f, 832.679f}, 28.608f, 227.511f},
        {arbol_tres, {-9.177f, 274.659f, 818.658f}, 38.924f, 12.738f},
        {arbol_cuatro, {-13.775f, 221.678f, 838.779f}, 32.938f, 151.589f},
        {arbol_uno, {27.336f, 255.577f, 843.091f}, 26.634f, 39.108f},
        {arbol_dos, {55.361f, 354.732f, 817.079f}, 26.073f, 298.662f},
        {arbol_tres, {52.236f, 298.054f, 834.717f}, 41.101f, 82.432f},
        {arbol_uno, {-859.505f, 71.178f, 168.537f}, 40.717f, 75.569f},
        {arbol_dos, {-929.192f, 21.447f, 89.307f}, 33.369f, 5.037f},
        {arbol_tres, {-901.106f, 44.495f, 146.092f}, 36.255f, 108.878f},
        {arbol_cuatro, {-918.528f, 35.746f, 67.393f}, 37.874f, 106.669f},
        {arbol_uno, {-888.235f, 54.451f, 135.032f}, 42.019f, 55.529f},
        {arbol_dos, {-913.797f, 32.359f, 121.099f}, 33.405f, 308.478f},
        {arbol_tres, {-924.956f, 24.940f, 58.099f}, 37.973f, 196.882f},
        {arbol_uno, {-897.830f, 69.526f, -719.998f}, 28.655f, 170.014f},
        {arbol_dos, {-921.765f, 4.896f, -735.816f}, 37.324f, 24.673f},
        {arbol_tres, {-889.266f, 62.070f, -753.093f}, 31.808f, 217.467f},
        {arbol_cuatro, {-907.741f, 60.835f, -673.099f}, 40.911f, 32.123f},
        {arbol_uno, {-886.757f, 65.498f, -768.366f}, 30.992f, 324.821f},
        {arbol_dos, {-918.365f, 1.546f, -748.568f}, 36.915f, 295.695f},
        {arbol_tres, {-903.564f, 25.410f, -759.710f}, 40.081f, 300.520f},
        {arbol_uno, {-711.519f, 119.656f, 234.756f}, 30.765f, 153.327f},
        {arbol_dos, {-644.174f, 65.605f, 233.436f}, 33.344f, 247.598f},
        {arbol_tres, {-625.124f, 50.392f, 232.291f}, 37.402f, 270.124f},
        {arbol_cuatro, {-685.863f, 120.456f, 194.600f}, 32.801f, 304.456f},
        {arbol_uno, {-649.371f, 76.145f, 211.450f}, 31.443f, 51.537f},
        {arbol_dos, {-636.753f, 39.986f, 276.011f}, 36.892f, 164.662f},
        {arbol_tres, {-663.213f, 91.062f, 192.816f}, 37.809f, 274.529f},
        {arbol_uno, {763.169f, 58.901f, 371.153f}, 37.873f, 229.197f},
        {arbol_dos, {804.117f, 109.359f, 397.032f}, 37.713f, 152.099f},
        {arbol_tres, {765.790f, 76.801f, 346.387f}, 31.391f, 84.819f},
        {arbol_cuatro, {755.301f, 74.485f, 349.724f}, 33.749f, 249.834f},
        {arbol_uno, {784.925f, 61.638f, 347.432f}, 29.444f, 206.787f},
        {arbol_dos, {773.193f, 53.028f, 379.477f}, 28.854f, 63.127f},
        {arbol_tres, {785.115f, 57.411f, 359.981f}, 37.105f, 54.362f},
        {arbol_uno, {837.743f, 70.353f, 849.046f}, 29.666f, 216.051f},
        {arbol_dos, {863.379f, 28.763f, 866.694f}, 39.418f, 171.635f},
        {arbol_tres, {870.409f, 29.531f, 826.747f}, 37.317f, 344.871f},
        {arbol_cuatro, {889.890f, 14.593f, 811.399f}, 34.203f, 95.921f},
        {arbol_uno, {872.502f, 32.023f, 790.839f}, 42.321f, 179.410f},
        {arbol_dos, {896.983f, 12.037f, 839.540f}, 32.966f, 247.880f},
        {arbol_tres, {884.398f, 20.491f, 867.057f}, 31.614f, 254.974f},
        {arbol_uno, {757.967f, 35.599f, 708.577f}, 38.341f, 241.805f},
        {arbol_dos, {798.798f, 91.445f, 688.518f}, 35.399f, 164.717f},
        {arbol_tres, {731.782f, 23.866f, 693.424f}, 26.300f, 211.871f},
        {arbol_cuatro, {759.170f, 18.540f, 733.606f}, 31.964f, 86.026f},
        {arbol_uno, {743.499f, 18.520f, 699.586f}, 37.530f, 39.780f},
        {arbol_dos, {722.444f, 28.922f, 695.118f}, 37.211f, 237.349f},
        {arbol_tres, {738.022f, 14.067f, 712.108f}, 28.677f, 143.960f},
        {arbol_tres, {-279.463f, 75.848f, -838.956f}, 39.853f, 120.215f},
        {arbol_cuatro, {805.164f, 197.013f, -227.898f}, 42.874f, 247.723f},
        {arbol_uno, {50.016f, 48.816f, -921.801f}, 32.433f, 221.460f},
        {arbol_dos, {657.145f, 97.254f, 33.968f}, 29.759f, 317.530f},
        {arbol_tres, {255.825f, 26.487f, -903.457f}, 41.721f, 313.310f},
        {arbol_cuatro, {738.650f, 132.706f, 253.647f}, 30.821f, 91.006f},
        {arbol_uno, {-553.552f, 30.564f, 703.600f}, 39.612f, 80.907f},
        {arbol_dos, {100.518f, 361.310f, 822.246f}, 41.713f, 254.924f},
        {arbol_tres, {765.493f, 115.354f, 278.106f}, 32.681f, 266.324f},
        {arbol_cuatro, {279.376f, 57.433f, 902.403f}, 31.361f, 312.599f},
        {arbol_uno, {908.908f, 81.582f, 110.936f}, 28.448f, 247.440f},
        {arbol_dos, {-761.178f, 360.660f, 79.806f}, 27.019f, 317.672f},
        {arbol_tres, {384.909f, 86.700f, -776.602f}, 25.532f, 51.353f},
        {arbol_cuatro, {850.412f, 206.933f, -433.737f}, 31.843f, 246.820f},
        {arbol_uno, {332.334f, 65.346f, -664.915f}, 41.815f, 114.460f},
        {arbol_dos, {-655.474f, 23.809f, 562.807f}, 30.880f, 21.135f},
        {arbol_tres, {-135.117f, 62.086f, -898.563f}, 27.907f, 117.658f},
        {arbol_cuatro, {-304.949f, 76.694f, -711.914f}, 39.772f, 63.990f},
        {arbol_uno, {64.188f, 117.496f, 644.304f}, 31.654f, 30.200f},
        {arbol_dos, {935.232f, 16.466f, -212.655f}, 39.014f, 297.555f},
        {arbol_tres, {-670.294f, 56.045f, 305.317f}, 35.004f, 216.510f},
        {arbol_cuatro, {-409.042f, 75.507f, -766.510f}, 42.804f, 207.244f},
        {arbol_uno, {811.119f, 149.547f, 493.372f}, 35.664f, 321.366f},
        {arbol_dos, {833.069f, 116.455f, -301.542f}, 32.649f, 236.885f},
        {arbol_tres, {-147.369f, 155.499f, 684.452f}, 38.406f, 46.990f},
        {arbol_cuatro, {-861.889f, 97.423f, -211.901f}, 39.161f, 73.147f},
        {casa_uno, {289.000f, -0.120f, 185.000f}, 23.200f, -2.741f},
        {casa_dos, {298.000f, -0.120f, 230.000f}, 24.400f, 3.284f},
        {casa_uno, {295.000f, -0.120f, 275.000f}, 23.200f, 1.057f},
        {casa_dos, {295.000f, -0.120f, 320.000f}, 24.400f, -3.797f},
        {casa_uno, {385.000f, -1.359f, 191.000f}, 23.200f, 183.380f},
        {casa_dos, {385.000f, -0.551f, 230.000f}, 24.400f, 177.825f},
        {casa_uno, {385.000f, -0.884f, 275.000f}, 23.200f, 181.866f},
        {casa_dos, {385.000f, -0.724f, 314.000f}, 24.400f, 181.229f},
        {casa_uno, {337.000f, -0.120f, 405.000f}, 23.200f, 177.600f},
        {casa_dos, {278.000f, -0.120f, 360.000f}, 24.400f, 58.992f},
        {casa_uno, {247.000f, -0.367f, 407.000f}, 23.200f, 54.328f},
        {casa_dos, {215.000f, -0.549f, 455.000f}, 24.400f, 51.794f},
        {casa_uno, {297.000f, -0.944f, 480.000f}, 23.200f, 235.616f},
        {casa_dos, {275.000f, -0.491f, 519.000f}, 24.400f, 235.478f},
        {casa_uno, {425.000f, -1.169f, 310.000f}, 23.200f, 98.762f},
        {casa_dos, {470.000f, -0.561f, 328.000f}, 24.400f, 103.191f},
        {casa_uno, {506.000f, -1.171f, 335.000f}, 23.200f, 98.737f},
        {casa_dos, {250.000f, -0.120f, 208.000f}, 24.400f, 180.400f},
        {casa_uno, {250.000f, -0.120f, 270.000f}, 23.200f, 182.707f},
        {casa_dos, {234.000f, -0.120f, 320.000f}, 24.400f, 5.402f},
        {casa_uno, {440.000f, -0.898f, 220.000f}, 23.200f, 266.723f},
        {casa_dos, {485.000f, -0.731f, 237.000f}, 24.400f, 5.471f},
        {casa_uno, {520.000f, -0.663f, 265.000f}, 23.200f, 90.184f},
        {casa_dos, {210.000f, -0.140f, 385.000f}, 24.400f, 178.339f},
        {casa_uno, {418.000f, -1.497f, 500.000f}, 23.200f, 88.186f},
        {casa_dos, {243.000f, -0.643f, 505.000f}, 24.400f, 269.233f},
        {arco_casas, {340.000f, -0.120f, 180.000f}, 25.000f, 0.000f},
        {arco_casas, {260.000f, -0.717f, 465.000f}, 25.000f, -32.000f},
        {arco_casas, {490.000f, -0.794f, 374.000f}, 25.000f, 77.000f},
    };
    decoracion.clear();
    decoracion.reserve(sizeof(distribucion) / sizeof(distribucion[0]) + 3);
    for (const auto& elemento : distribucion)
    {
        glm::mat4 matriz = glm::translate(
            glm::mat4(1), glm::vec3(elemento.posicion.x,
                                    altura_suelo + elemento.posicion.y +
                                        AlturaSuelo(elemento.posicion.x, elemento.posicion.z),
                                    elemento.posicion.z));
        matriz = glm::scale(glm::rotate(matriz, glm::radians(elemento.giro), glm::vec3(0, 1, 0)),
                            glm::vec3(elemento.escala));
        decoracion.push_back({elemento.tipo, matriz});
        if (elemento.tipo == arbol_portal)
            decoracion.push_back({portal_azul, matriz});
        if (elemento.tipo == domo_cristal)
            decoracion.push_back({bordes_domo, matriz});
    }
}

//>>>>>>>>>>>>>>>>>>>>>> Distribucion: faros de ambas pistas >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CrearFaros()
{
    apoyos_faroles_elevados.clear();
    // Conservar el 60 % de los faros de cada pista, repartidos por longitud.
    matrices_postes.clear();
    float distancia_faros = 48.0f;
    int cantidad_anterior_espiral = 0;
    std::vector<float> tiempos_faros{0.0f};
    std::vector<float> longitudes_faros{0.0f};
    glm::vec3 anterior = RecorridoEspiral::Punto(0.0f);
    for (float tiempo = 0.05f; tiempo < RecorridoEspiral::tiempo_total - RecorridoEspiral::tiempo_caida;
         tiempo += 0.05f)
    {
        glm::vec3 punto = RecorridoEspiral::Punto(tiempo);
        float segmento = glm::length(punto - anterior);
        distancia_faros += segmento;
        tiempos_faros.push_back(tiempo);
        longitudes_faros.push_back(longitudes_faros.back() + segmento);
        anterior = punto;
        if (distancia_faros < 48.0f)
            continue;
        distancia_faros = 0.0f;
        ++cantidad_anterior_espiral;
    }
    int cantidad_espiral = std::max(1, int(std::lround(cantidad_anterior_espiral * 0.6f)));
    for (int faro = 0; faro < cantidad_espiral; ++faro)
    {
        float distancia = (faro + 0.5f) * longitudes_faros.back() / cantidad_espiral;
        size_t i = std::lower_bound(longitudes_faros.begin(), longitudes_faros.end(), distancia) -
                   longitudes_faros.begin();
        i = std::max(size_t(1), i);
        float mezcla =
            (distancia - longitudes_faros[i - 1]) / (longitudes_faros[i] - longitudes_faros[i - 1]);
        float tiempo = glm::mix(tiempos_faros[i - 1], tiempos_faros[i], mezcla);
        glm::vec3 exterior = -RecorridoEspiral::Lateral(tiempo, 0.0f);
        glm::vec3 punto_borde = PuntoTablero(tiempo, -21.5f) - glm::vec3(0, 0.35f, 0);
        // Retirar el faro de la ultima vuelta que atraviesa el puente de salida.
        if (FaroInvadeTableroSuperior(punto_borde, -exterior))
        {
            continue;
        }
        matrices_postes.push_back(glm::scale(
            glm::mat4(glm::vec4(glm::cross(glm::vec3(0, 1, 0), exterior), 0), glm::vec4(0, 1, 0, 0),
                      glm::vec4(exterior, 0), glm::vec4(punto_borde + glm::vec3(0, altura_suelo, 0), 1)),
            glm::vec3(14.0f)));
    }
    int cantidad_anterior_inferior = 0;
    for (float d = 40.0f; d < longitud_circuito; d += 160.0f)
        ++cantidad_anterior_inferior;
    int cantidad_inferior = std::max(1, int(std::lround(cantidad_anterior_inferior * 0.6f)));
    for (int faro = 0; faro < cantidad_inferior; ++faro)
    {
        float distancia = (faro + 0.5f) * longitud_circuito / cantidad_inferior;
        auto it = std::lower_bound(distancias.begin(), distancias.end(), distancia);
        size_t i = std::max(size_t(1), size_t(it - distancias.begin()));
        glm::vec3 direccion = glm::normalize(RecorridoNaranja::puntos[i] - RecorridoNaranja::puntos[i - 1]);
        glm::vec3 lado = glm::normalize(glm::cross(direccion, glm::vec3(0, 1, 0)));
        float t = (distancia - distancias[i - 1]) / (distancias[i] - distancias[i - 1]);
        glm::vec3 centro = glm::mix(RecorridoNaranja::puntos[i - 1], RecorridoNaranja::puntos[i], t);
        glm::vec3 punto = centro;
        glm::vec3 lado_elegido = lado;
        float mejor_holgura = -1.0f;
        bool separado = false;
        for (float retiro = 110.0f; retiro <= 360.0f && !separado; retiro += 10.0f)
            for (int orientacion = 0; orientacion < 16; ++orientacion)
            {
                float angulo = orientacion * (glm::two_pi<float>() / 16.0f);
                glm::vec3 retiro_lateral =
                    lado * std::cos(angulo) + glm::cross(glm::vec3(0, 1, 0), lado) * std::sin(angulo);
                glm::vec3 candidato = centro + retiro_lateral * retiro;
                float holgura = DistanciaCircuitoPlanta(candidato);
                if (holgura > mejor_holgura)
                {
                    mejor_holgura = holgura;
                    punto = candidato;
                    lado_elegido = retiro_lateral;
                }
                // 18 unidades del mundo: semiancho maximo 10.8, cabeza 5.2 y margen 2.
                if (holgura >= 100.0f)
                {
                    separado = true;
                    break;
                }
            }
        bool elevado = centro.y * escala_pista > 0.75f;
        if (elevado)
        {
            glm::vec2 bordes = BordesEnSeccion(centro, direccion, lado);
            // La base queda fuera del carril, sobre una mensula unida al borde real.
            punto = centro + lado * (bordes.y + 1.35f / escala_pista);
            lado_elegido = lado;
        }
        glm::mat4 matriz_poste = glm::scale(
            glm::mat4(glm::vec4(glm::cross(glm::vec3(0, 1, 0), lado_elegido), 0), glm::vec4(0, 1, 0, 0),
                      glm::vec4(lado_elegido, 0), matriz_pista * glm::vec4(punto, 1)),
            glm::vec3(10.0f));
        matrices_postes.push_back(matriz_poste);
        if (elevado)
            apoyos_faroles_elevados.push_back(matriz_poste);
    }
    // Las posiciones de las luminarias no cambian durante la animacion.
    posiciones_luces_postes.clear();
    posiciones_luces_postes.reserve(matrices_postes.size());
    for (const auto& matriz : matrices_postes)
        posiciones_luces_postes.push_back(glm::vec3(matriz * glm::vec4(0, 1.37f, -0.36f, 1)));
}

//>>>>>>>>>>>>>>>>>>>>>> Luces reales: dos torres y dos domos >>>>>>>>>
void EscenaPlanetaNaranja::CrearIluminacion()
{
    posiciones_arquitectura[0] =
        posicion_torre_blanca + glm::vec3(0, altura_suelo + .02f + 83.758304f * 4.6f, 0);
    posiciones_arquitectura[1] =
        posicion_torre_control + glm::vec3(0, altura_suelo + (410.291962f + 116.374428f) * .8f, 0);
    int indice = 2;
    for (const auto& objeto : decoracion)
        if (objeto.tipo == domo_cristal && indice < 4)
            posiciones_arquitectura[indice++] = glm::vec3(objeto.matriz * glm::vec4(0, .82f, 0, 1));
}

//>>>>>>>>>>>>>>>>>>>>>> Mismo ciclo de 120 segundos del planeta azul >>
void EscenaPlanetaNaranja::ActualizarIluminacion(float dt)
{
    tiempo_dia = std::fmod(tiempo_dia + dt, IluminacionAmbiental::duracion_dia);
    const float dia = IluminacionAmbiental::FactorDia(tiempo_dia);
    es_noche = dia < .5f;
    skybox = es_noche ? cielo_noche : cielo_dia;
    const glm::vec3 color = glm::mix(glm::vec3(.72f, .78f, .95f), glm::vec3(1, .79f, .60f), dia);
    luz_sol = DirectionalLight(color.r, color.g, color.b, glm::mix(.27f, .48f, dia),
                               glm::mix(.27f, .85f, dia), -.45f, .85f, -.25f);
    for (int i = 0; i < 4; ++i)
    {
        const auto& p = posiciones_arquitectura[i];
        luces_arquitectura[i] =
            PointLight(1, .91f, .76f, 0, i < 2 ? (es_noche ? 3.2f : 0.0f) : (es_noche ? 2.8f : 1.2f), p.x,
                       p.y, p.z, 1, .04f, .0015f);
    }
}

//>>>>>>>>>>>>>>>>>>>>>> Halos pegados a suelo y tableros reales >>>>>>
void EscenaPlanetaNaranja::CrearHalos()
{
    textura_halo = Texture("assets/planeta_naranja/texturas/halo_faro.png");
    textura_halo.LoadTextureA();
    textura_halo.UseTexture();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const GLfloat borde[] = {0, 0, 0, 0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borde);
    std::vector<TrianguloPista> superficies;
    auto agregar = [&](const std::vector<TrianguloPista>& triangulos, const glm::mat4& padre) {
        for (const auto& tri : triangulos)
        {
            TrianguloPista mundo;
            for (int k = 0; k < 3; ++k)
                mundo[k] = glm::vec3(padre * glm::vec4(tri[k], 1));
            if (std::abs(glm::cross(mundo[1] - mundo[0], mundo[2] - mundo[0]).y) > .0001f)
                superficies.push_back(mundo);
        }
    };
    agregar(secciones_pista, matriz_pista);
    const glm::mat4 piso = glm::translate(glm::mat4(1), glm::vec3(0, altura_suelo, 0));
    agregar(secciones_espiral, piso);
    agregar(LeerSeccionesPista("assets/planeta_naranja/modelos/suelo.obj"), piso);
    // Indice espacial estatico: cada faro consulta solo los triangulos de sus celdas.
    std::map<std::pair<int, int>, std::vector<size_t>> celdas;
    for (size_t i = 0; i < superficies.size(); ++i)
    {
        const auto& t = superficies[i];
        const int min_x = int(std::floor(std::min({t[0].x, t[1].x, t[2].x}) / 32));
        const int max_x = int(std::floor(std::max({t[0].x, t[1].x, t[2].x}) / 32));
        const int min_z = int(std::floor(std::min({t[0].z, t[1].z, t[2].z}) / 32));
        const int max_z = int(std::floor(std::max({t[0].z, t[1].z, t[2].z}) / 32));
        for (int x = min_x; x <= max_x; ++x)
            for (int z = min_z; z <= max_z; ++z)
                celdas[{x, z}].push_back(i);
    }
    std::vector<GLfloat> vertices;
    std::vector<unsigned int> indices;
    for (const auto& luz : posiciones_luces_postes)
    {
        std::vector<size_t> candidatos;
        for (int x = int(std::floor((luz.x - 17) / 32)); x <= int(std::floor((luz.x + 17) / 32)); ++x)
            for (int z = int(std::floor((luz.z - 17) / 32)); z <= int(std::floor((luz.z + 17) / 32)); ++z)
            {
                auto celda = celdas.find({x, z});
                if (celda != celdas.end())
                    candidatos.insert(candidatos.end(), celda->second.begin(), celda->second.end());
            }
        std::sort(candidatos.begin(), candidatos.end());
        candidatos.erase(std::unique(candidatos.begin(), candidatos.end()), candidatos.end());

        float altura = -1.0e9f;
        glm::vec3 normal(0, 1, 0);
        for (size_t indice : candidatos)
        {
            const auto& tri = superficies[indice];
            const glm::vec3 a = tri[1] - tri[0], b = tri[2] - tri[0], q = luz - tri[0];
            const float det = a.x * b.z - a.z * b.x;
            if (std::abs(det) < .000001f)
                continue;
            const float u = (q.x * b.z - q.z * b.x) / det;
            const float v = (a.x * q.z - a.z * q.x) / det;
            if (u < 0 || v < 0 || u + v > 1)
                continue;
            const float y = tri[0].y + a.y * u + b.y * v;
            if (y < luz.y - .3f && y > altura && y > luz.y - 35)
            {
                altura = y;
                normal = glm::normalize(glm::cross(a, b));
                if (normal.y < 0)
                    normal = -normal;
            }
        }
        if (altura < -1.0e8f || normal.y < .25f)
            continue;
        const float radio = glm::clamp((luz.y - altura) * .62f, 5.2f, 17.0f);
        for (size_t indice : candidatos)
        {
            const auto& tri = superficies[indice];
            if (std::max({tri[0].x, tri[1].x, tri[2].x}) < luz.x - radio ||
                std::min({tri[0].x, tri[1].x, tri[2].x}) > luz.x + radio ||
                std::max({tri[0].z, tri[1].z, tri[2].z}) < luz.z - radio ||
                std::min({tri[0].z, tri[1].z, tri[2].z}) > luz.z + radio)
                continue;
            const glm::vec3 centro = (tri[0] + tri[1] + tri[2]) / 3.0f;
            const float plano =
                altura - (normal.x * (centro.x - luz.x) + normal.z * (centro.z - luz.z)) / normal.y;
            if (std::abs(centro.y - plano) > .65f)
                continue;
            for (const auto& vertice : tri)
            {
                const glm::vec3 p = vertice + normal * .035f;
                indices.push_back(static_cast<unsigned int>(vertices.size() / 8));
                vertices.insert(vertices.end(),
                                {p.x, p.y, p.z, (vertice.x - luz.x) / (radio * 2) + .5f,
                                 (vertice.z - luz.z) / (radio * 2) + .5f, normal.x, normal.y, normal.z});
            }
        }
    }
    if (!indices.empty())
    {
        malla_halos = new MeshModel();
        malla_halos->CreateMeshModel(vertices.data(), indices.data(),
                                     static_cast<unsigned int>(vertices.size()),
                                     static_cast<unsigned int>(indices.size()));
    }
}

//>>>>>>>>>>>>>>>>>>>>>> Luz horneada visual, sin luces puntuales >>>>>
void EscenaPlanetaNaranja::DibujarHalos(const ContextoRender& ctx)
{
    if (!es_noche || !malla_halos)
        return;
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
    glUniform1i(glGetUniformLocation(programa, "modo_efecto"), 4);
    glUniform1f(glGetUniformLocation(programa, "opacidad_particula"), .55f);
    glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));
    glUniform3f(ctx.u_color, 1.25f, 1.12f, .86f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2, -4);
    textura_halo.UseTexture();
    malla_halos->RenderMeshModel();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUniform1i(glGetUniformLocation(programa, "modo_efecto"), 0);
    glUniform1f(glGetUniformLocation(programa, "opacidad_particula"), 1);
    glUniform3f(ctx.u_color, 1, 1, 1);
}

//>>>>>>>>>>>>>>>>>>>>>> Animacion de la escena >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::Actualizar(float dt, Window& ventana)
{
    ActualizarIluminacion(dt);
    motor1 = ventana.getMotor1Encendido();
    motor2 = ventana.getMotor2Encendido();
    if (ventana.getVistaGeneralSolicitada())
        camara = Camera(glm::vec3(700.0f, altura_suelo + 700.0f, 1100.0f), glm::vec3(0, 1, 0), -126.0f,
                        -25.0f, 150.0f, 0.3f);
    ActualizarNaves(dt);
    tiempo_rocky += dt;
    cohete_auxiliar.Actualizar(dt,ventana.TeclaPresionada(GLFW_KEY_F));
    ActualizarObjetosAereos(dt);
}

//>>>>>>>>>>>>>>>>>>>>>> Jerarquias completas: pistas -> naves -> motores >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::ActualizarNaves(float dt)
{
    // Padre 1: pista inferior -> trayectoria -> orientacion y escala de la nave.

    // Distancia real por segundo; conserva el sobrante al pasar de segmento.
    avance_nave = std::fmod(avance_nave + (16.2f / escala_pista) * dt, longitud_circuito);
    const float semilargo = 13.0f * escala_nave;
    glm::vec3 centro = PuntoPista(avance_nave);
    glm::vec3 adelante =
        glm::normalize(PuntoPista(avance_nave + semilargo) - PuntoPista(avance_nave - semilargo));
    glm::vec3 derecha = glm::normalize(glm::cross(adelante, glm::vec3(0, 1, 0)));
    glm::vec3 arriba = glm::normalize(glm::cross(derecha, adelante));
    // Anticipar la altura de la rampa debajo de toda la longitud del casco.
    float holgura = 0.0f;
    for (int k = -8; k <= 8; ++k)
        holgura =
            std::max(holgura, glm::dot(PuntoPista(avance_nave + semilargo * k / 8.0f) - centro, arriba));
    // El OBJ tiene los motores en +X: la nariz mira a -X.
    matriz_nave = glm::scale(matriz_pista * glm::mat4(glm::vec4(-adelante, 0), glm::vec4(arriba, 0),
                                                      glm::vec4(-derecha, 0),
                                                      glm::vec4(centro + arriba * (4.0f + holgura), 1)),
                             glm::vec3(escala_nave));

    // Padre 2: espiral -> trayectoria -> orientacion y escala de la nave.

    tiempo_espiral = std::fmod(tiempo_espiral + 1.15f * dt, RecorridoEspiral::tiempo_total);
    RecorridoEspiral::Estado estado = RecorridoEspiral::Evaluar(tiempo_espiral);
    matriz_nave_espiral = glm::scale(
        glm::mat4(glm::vec4(-estado.adelante, 0), glm::vec4(estado.arriba, 0),
                  glm::vec4(-estado.lateral, 0),
                  glm::vec4(estado.posicion + estado.arriba * 1.2f + glm::vec3(0, altura_suelo, 0), 1)),
        glm::vec3(0.198f));

    // Cada fila conserva su carril y su separacion; la velocidad varia entre carriles.
    tiempo_carreras += dt;
    for (int i = 0; i < 19; ++i)
    {
        const auto& rival = configuraciones_rivales[i];
        if (rival.espiral)
        {
            float tiempo = float(std::fmod(tiempo_carreras * 1.15f * rival.velocidad +
                                               rival.fase * RecorridoEspiral::tiempo_total,
                                           RecorridoEspiral::tiempo_total));
            auto estado_rival = RecorridoEspiral::Evaluar(tiempo, rival.carril);
            float intervalo_casco =
                3.5f * rival.escala /
                glm::length((RecorridoEspiral::PuntoCarril(tiempo + .01f, rival.carril) -
                             RecorridoEspiral::PuntoCarril(tiempo - .01f, rival.carril)) /
                            .02f);
            float holgura_rival = 0.0f;
            for (int k = -8; k <= 8; ++k)
                holgura_rival = std::max(holgura_rival,
                                         glm::dot(RecorridoEspiral::PuntoCarril(
                                                      tiempo + intervalo_casco * k / 8.0f, rival.carril) -
                                                      estado_rival.posicion,
                                                  estado_rival.arriba));
            matrices_rivales[i] = glm::scale(
                glm::mat4(glm::vec4(-estado_rival.lateral, 0), glm::vec4(estado_rival.arriba, 0),
                          glm::vec4(estado_rival.adelante, 0),
                          glm::vec4(estado_rival.posicion + estado_rival.arriba * (1.35f + holgura_rival) +
                                        glm::vec3(0, altura_suelo, 0),
                                    1)),
                glm::vec3(rival.escala));
        }
        else
        {
            float distancia = float(std::fmod(tiempo_carreras * (16.2f / escala_pista) * rival.velocidad +
                                                  rival.fase * longitud_circuito,
                                              longitud_circuito));
            glm::vec3 centro_rival = RecorridoNaranja::PuntoCarril(distancia, distancias, rival.carril);
            glm::vec3 frente_rival =
                glm::normalize(RecorridoNaranja::PuntoCarril(distancia + 6, distancias, rival.carril) -
                               RecorridoNaranja::PuntoCarril(distancia - 6, distancias, rival.carril));
            glm::vec3 derecha_rival = glm::normalize(glm::cross(glm::vec3(0, 1, 0), frente_rival));
            glm::vec3 arriba_rival = glm::normalize(glm::cross(frente_rival, derecha_rival));
            float semilargo_rival = 3.5f * rival.escala / escala_pista;
            float holgura_rival = 0.0f;
            for (int k = -8; k <= 8; ++k)
                holgura_rival =
                    std::max(holgura_rival,
                             glm::dot(RecorridoNaranja::PuntoCarril(distancia + semilargo_rival * k / 8.0f,
                                                                    distancias, rival.carril) -
                                          centro_rival,
                                      arriba_rival));
            glm::vec3 posicion_rival = glm::vec3(
                matriz_pista * glm::vec4(centro_rival + arriba_rival * (4.0f + holgura_rival), 1));
            matrices_rivales[i] =
                glm::scale(glm::mat4(glm::vec4(derecha_rival, 0), glm::vec4(arriba_rival, 0),
                                     glm::vec4(frente_rival, 0), glm::vec4(posicion_rival, 1)),
                           glm::vec3(rival.escala));
        }
        matrices_rivales[i] =
            glm::rotate(matrices_rivales[i], glm::radians(giros_rivales[rival.modelo]), glm::vec3(0, 1, 0));
    }

    // Hijos de ambas naves: salida del motor -> orientacion -> escala del fuego.
    for (int indice_nave = 0; indice_nave < 2; ++indice_nave)
        for (int motor = 0; motor < 2; ++motor)
        {
            glm::mat4 fuego_motor = glm::translate(indice_nave == 0 ? matriz_nave : matriz_nave_espiral,
                                                   glm::vec3(13, 0, motor == 0 ? 4 : -4));
            fuego_motor = glm::rotate(fuego_motor, glm::radians(90.0f), glm::vec3(0, 1, 0));
            matrices_fuegos[indice_nave][motor] = glm::scale(fuego_motor, glm::vec3(4, 4, 5.4f));
        }
}

//>>>>>>>>>>>>>>>>>>>>>> Jerarquias completas: curvas -> cuerpos, rotores y emisores >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::ActualizarObjetosAereos(float dt)
{
    tiempo_aereo = std::fmod(tiempo_aereo + dt, RecorridoAereo::periodo_comun);
    angulo_aspas = std::fmod(angulo_aspas + 1200.0f * dt, 360.0f);
    for (int i = 0; i < 3; ++i)
    {
        matrices_asteroides[i] = MatrizAsteroide(tiempo_aereo, i);
        // Curva -> marco padre -> giro y escala del cuerpo.
        matrices_cuerpos_asteroides[i] =
            glm::scale(glm::rotate(matrices_asteroides[i], glm::two_pi<float>() * tiempo_aereo / 28.0f + i,
                                   glm::normalize(glm::vec3(.4f, 1, .25f))),
                       glm::vec3(diametro_asteroide));
        // El fuego hereda el marco de vuelo, sin heredar el volteo del cuerpo.
        matrices_estelas_asteroides[i] =
            glm::scale(glm::translate(matrices_asteroides[i], glm::vec3(-diametro_asteroide * 0.28f, 0, 0)),
                       glm::vec3(180, 60, 60));
    }
    for (int dron = 0; dron < 5; ++dron)
    {
        matrices_drones[dron] = MatrizDron(tiempo_aereo, dron);
        // Curva -> dron -> pivote de cada rotor -> giro de las cuatro palas.
        for (int rotor = 0; rotor < 2; ++rotor)
            matrices_rotores[dron][rotor] =
                glm::rotate(glm::translate(matrices_drones[dron], pivotes_rotores[rotor]),
                            glm::radians(angulo_aspas * (rotor == 0 ? 1.0f : -1.0f)), glm::vec3(0, 1, 0));
    }
    for (auto& particula : humo_aereo)
        particula.edad += dt;
    emision_humo_aereo += dt;
    while (emision_humo_aereo >= intervalo_humo_aereo)
    {
        emision_humo_aereo -= intervalo_humo_aereo;
        if (emision_humo_aereo >= vida_humo_aereo)
            continue;
        for (int i = 0; i < 3; ++i)
        {
            auto& particula = humo_aereo[siguiente_humo_aereo];
            glm::mat4 emisor = glm::translate(MatrizAsteroide(tiempo_aereo - emision_humo_aereo, i),
                                              glm::vec3(-diametro_asteroide * 0.60f, 0, 0));
            particula.posicion = glm::vec3(emisor[3]);
            particula.edad = emision_humo_aereo;
            particula.tamano = 24.0f + i * 3.0f;
            particula.asteroide = i;
            siguiente_humo_aereo = (siguiente_humo_aereo + 1) % max_humo_aereo;
        }
    }
}

//>>>>>>>>>>>>>>>>>>>>>> Dibujo de la escena >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::Dibujar(const ContextoRender& ctx)
{
    GLint programa_ciclo = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programa_ciclo);
    glUniform1f(glGetUniformLocation(programa_ciclo, "factor_noche"), es_noche ? 1.0f : 0.0f);
    glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE,
                       glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(0, altura_suelo, 0))));
    glUniform3f(ctx.u_color, 1.0f, 1.0f, 1.0f);
    glUniform1f(ctx.u_specular_intensity, 0.08f);
    glUniform1f(ctx.u_shininess, 8.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    suelo.RenderModel();
    glDisable(GL_BLEND);

    // Relieve periferico, con tonos calidos y alturas que dejan visible el cielo.
    GLint programa_paisaje = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programa_paisaje);
    GLint uniform_relieve = glGetUniformLocation(programa_paisaje, "relieve_vibrante");
    glUniform1i(uniform_relieve, 1);
    glUniform3f(ctx.u_color, es_noche ? 1.03f : 1.40f, es_noche ? .79f : .82f, es_noche ? .65f : .52f);
    for (const auto& elemento : montanas_perifericas)
    {
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(elemento.matriz));
        montanas[elemento.modelo].RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    }
    for (const auto& matriz : rocas_perifericas)
    {
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matriz));
        roca_periferica.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    }
    glUniform1i(uniform_relieve, 0);
    glUniform3f(ctx.u_color, 1.0f, 1.0f, 1.0f);

    //>>>>>>>>>>>>>>>>>>>>>> Pista completa >>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matriz_pista));
    glUniform1f(ctx.u_specular_intensity, 0.25f);
    glUniform1f(ctx.u_shininess, 32.0f);
    pista.RenderModel();

    //>>>>>>>>>>>>>>>>>>>>>> Pista espiral y pilares >>>>>>>>>>>>>>>>>>>>>
    glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE,
                       glm::value_ptr(glm::translate(glm::mat4(1.0f), glm::vec3(0, altura_suelo, 0))));
    pista_espiral.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    soportes_y_senalizacion.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);

    // Torres en los claros interiores, con bases fuera del ancho de ambas pistas.
    glUniformMatrix4fv(
        ctx.u_model, 1, GL_FALSE,
        glm::value_ptr(glm::scale(
            glm::translate(glm::mat4(1), posicion_torre_blanca + glm::vec3(0, altura_suelo + 0.02f, 0)),
            glm::vec3(2.475f, 3.45f, 2.475f))));
    torre_comunicaciones.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);

    glUniformMatrix4fv(
        ctx.u_model, 1, GL_FALSE,
        glm::value_ptr(
            glm::scale(glm::translate(glm::mat4(1), posicion_torre_control +
                                                        glm::vec3(0, altura_suelo + 410.292f * 0.8f, 0)),
                       glm::vec3(0.5f, 0.8f, 0.5f))));
    torre_control.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);

    //>>>>>>>>>>>>>>>>>>>>>> Postes del borde exterior >>>>>>>>>>>>>>>>>>
    for (const glm::mat4& matriz_poste : matrices_postes)
    {
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matriz_poste));
        poste_luz.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    }
    for (const auto& matriz : apoyos_faroles_elevados)
    {
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matriz));
        soporte_faro_lateral.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    }

    DibujarDecoracion(ctx, false);
    DibujarRocky(ctx);
    cohete_auxiliar.Dibujar(ctx,{330,altura_suelo+AlturaSuelo(330,-470),-470},es_noche ? 1.0f : 0.0f);
    DibujarNaves(ctx);
    DibujarObjetosAereos(ctx);
    DibujarHalos(ctx);
    DibujarDecoracion(ctx, true);
    if (es_noche)
    {
        DibujarBrillos(ctx, nave, matriz_nave, textura_halo);
        DibujarBrillos(ctx, nave, matriz_nave_espiral, textura_halo);
        for (size_t i = 0; i < sizeof(configuraciones_rivales) / sizeof(configuraciones_rivales[0]); ++i)
            DibujarBrillos(ctx, rivales[configuraciones_rivales[i].modelo], matrices_rivales[i],
                           textura_halo);
        for (const auto& padre : matrices_drones)
            DibujarBrillos(ctx, dron_explorador, padre, textura_halo, .85f);
        DibujarBrillos(ctx, torre_comunicaciones,
                       glm::scale(glm::translate(glm::mat4(1), posicion_torre_blanca +
                                                                   glm::vec3(0, altura_suelo + .02f, 0)),
                                  glm::vec3(3.3f, 4.6f, 3.3f)),
                       textura_halo);
        DibujarBrillos(
            ctx, torre_control,
            glm::scale(glm::translate(glm::mat4(1), posicion_torre_control +
                                                        glm::vec3(0, altura_suelo + 410.291962f * .8f, 0)),
                       glm::vec3(.8f)),
            textura_halo);
    }
    glUniform1f(glGetUniformLocation(programa_ciclo, "factor_noche"), 1.0f);
}

//>>>>>>>>>>>>>>>>>>>>>> Decoracion opaca y cristales ordenados >>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::DibujarDecoracion(const ContextoRender& ctx, bool transparente)
{
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
    glUniform1i(glGetUniformLocation(programa, "modo_efecto"), 0);
    glUniform1i(glGetUniformLocation(programa, "relieve_vibrante"), 0);
    glUniform3f(ctx.u_color, 1, 1, 1);
    glUniform1i(glGetUniformLocation(programa, "recorte_alfa"), 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    std::vector<const DecoracionEstatica*> orden;
    for (const auto& elemento : decoracion)
        if ((elemento.tipo == domo_cristal || elemento.tipo == portal_azul) == transparente)
            orden.push_back(&elemento);
    if (transparente)
    {
        const glm::vec3 ojo = ctx.pos_camara;
        std::sort(orden.begin(), orden.end(), [&ojo](const auto* a, const auto* b) {
            glm::vec3 distancia_a = glm::vec3(a->matriz[3]) - ojo;
            glm::vec3 distancia_b = glm::vec3(b->matriz[3]) - ojo;
            return glm::dot(distancia_a, distancia_a) > glm::dot(distancia_b, distancia_b);
        });
        glDepthMask(GL_FALSE);
    }
    for (const auto* elemento : orden)
    {
        glUniform1f(glGetUniformLocation(programa, "opacidad_aerea"),
                    elemento->tipo == portal_azul ? .55f : (elemento->tipo == domo_cristal ? .32f : 1.0f));
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(elemento->matriz));
        if (elemento->tipo == domo_cristal)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        decorativos[elemento->tipo].RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
        glDisable(GL_CULL_FACE);
    }
    glUniform1f(glGetUniformLocation(programa, "opacidad_aerea"), 1);
    glUniform1i(glGetUniformLocation(programa, "recorte_alfa"), 0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

//>>>>>>>>>>>>>>>>>>>>>> Dibujo de las naves y sus motores >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::DibujarNaves(const ContextoRender& ctx)
{
    // Padres: nave del circuito inferior y nave de la espiral.
    glUniform1f(ctx.u_specular_intensity, 0.6f);
    glUniform1f(ctx.u_shininess, 64.0f);
    for (const auto& raiz_nave : {matriz_nave, matriz_nave_espiral})
    {
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(raiz_nave));
        nave.RenderModel();
    }

    for (int i = 0; i < 19; ++i)
    {
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matrices_rivales[i]));
        rivales[configuraciones_rivales[i].modelo].RenderModel(true, ctx.u_specular_intensity,
                                                               ctx.u_shininess);
    }

    // Hijos: cada fuego hereda la transformacion completa de su nave.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glUniform3f(ctx.u_color, 2, 2, 2);
    glUniform1f(ctx.u_specular_intensity, 0);
    fuego->UseTexture();
    for (int motor = 0; motor < 2; ++motor)
    {
        if (!(motor == 0 ? motor1 : motor2))
            continue;
        for (int indice_nave = 0; indice_nave < 2; ++indice_nave)
        {
            glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE,
                               glm::value_ptr(matrices_fuegos[indice_nave][motor]));
            malla_fuego->RenderMeshModel();
        }
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glUniform3f(ctx.u_color, 1, 1, 1);
}

//>>>>>>>>>>>>>>>>>>>>>> Dibujo de drones, asteroides y efectos >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::DibujarObjetosAereos(const ContextoRender& ctx)
{
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
    GLint offset = glGetUniformLocation(programa, "desplazamiento_textura");
    GLint modo = glGetUniformLocation(programa, "modo_efecto");
    GLint opacidad = glGetUniformLocation(programa, "opacidad_particula");
    GLint visibilidad = glGetUniformLocation(programa, "opacidad_aerea");
    GLint normales = glGetUniformLocation(programa, "normales_geometricas");
    glUniform1i(modo, 0);
    glUniform2f(offset, 0, 0);
    glUniform1f(visibilidad, 1);
    glUniform3f(ctx.u_color, 1, 1, 1);
    for (int dron = 0; dron < 5; ++dron)
    {
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matrices_drones[dron]));
        dron_explorador.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
        textura_aspas->UseTexture();
        glUniform1i(normales, 1);
        glUniform1f(ctx.u_specular_intensity, 0.8f);
        glUniform1f(ctx.u_shininess, 96.0f);
        for (int rotor = 0; rotor < 2; ++rotor)
        {
            glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matrices_rotores[dron][rotor]));
            malla_aspas_dron->RenderMeshModel();
        }
        glUniform1i(normales, 0);
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < 3; ++i)
    {
        float visible = VisibilidadAsteroide(matrices_asteroides[i]);
        if (visible < 0.001f)
            continue;
        glUniform1f(visibilidad, visible);
        glUniform3f(ctx.u_color, i == 0 ? 0.60f : 1.0f, i == 0 ? 0.48f : 1.0f, i == 0 ? 0.38f : 1.0f);
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matrices_cuerpos_asteroides[i]));
        asteroides[i].RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    }
    glUniform1f(visibilidad, 1);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glUniform1i(modo, 1);
    glUniform3f(ctx.u_color, 1, 1, 1);
    textura_fuego_asteroide->UseTexture();
    for (int i = 0; i < 3; ++i)
    {
        float visible = VisibilidadAsteroide(matrices_asteroides[i]);
        if (visible < 0.001f)
            continue;
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(matrices_estelas_asteroides[i]));
        glUniform2f(offset, 0, std::fmod(-tiempo_aereo * 1.25f + i * .23f, 1.0f));
        glUniform1f(opacidad, visible);
        malla_estela_asteroide->RenderMeshModel();
    }

    // Como dibujaHumo() en P10: planos frente a la camara, ordenados de lejos a cerca.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(modo, 2);
    glUniform2f(offset, 0, 0);
    textura_humo->UseTexture();
    const glm::mat4& vista = ctx.vista;
    std::vector<const ParticulaHumoAereo*> visibles;
    for (const auto& particula : humo_aereo)
        if (particula.edad < vida_humo_aereo)
            visibles.push_back(&particula);
    std::sort(visibles.begin(), visibles.end(), [&vista](const auto* a, const auto* b) {
        return (vista * glm::vec4(a->posicion, 1)).z < (vista * glm::vec4(b->posicion, 1)).z;
    });
    for (const auto* particula : visibles)
    {
        glm::mat4 plano = glm::transpose(glm::mat4(glm::mat3(vista)));
        plano[3] = glm::vec4(particula->posicion + glm::vec3(0, 0.7f * particula->edad, 0), 1);
        plano = glm::scale(plano, glm::vec3(particula->tamano + 10.0f * particula->edad));
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(plano));
        glUniform1f(opacidad, 0.55f * (1.0f - particula->edad / vida_humo_aereo) *
                                  VisibilidadAsteroide(matrices_asteroides[particula->asteroide]));
        malla_humo_aereo->RenderMeshModel();
    }
    glUniform1i(modo, 0);
    glUniform2f(offset, 0, 0);
    glUniform1f(visibilidad, 1);
    glUniform1f(opacidad, 1);
    glUniform3f(ctx.u_color, 1, 1, 1);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

//>>>>>>>>>>>>>>>>>>>>>> Consulta: nave inferior >>>>>>>>>>>>>>>>>>>
glm::mat4 EscenaPlanetaNaranja::ObtenerMatrizNave() const
{
    return matriz_nave;
}

//>>>>>>>>>>>>>>>>>>>>>> Consulta: nave espiral >>>>>>>>>>>>>>>>>>>
glm::mat4 EscenaPlanetaNaranja::ObtenerMatrizNaveEspiral() const
{
    return matriz_nave_espiral;
}

//>>>>>>>>>>>>>>>>>>>>>> Consulta: luces cercanas a la camara >>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>> Liberacion de recursos >>>>>>>>>>>>>>>>>>>
EscenaPlanetaNaranja::~EscenaPlanetaNaranja()
{
    for (auto& pieza : rocky_saludo) pieza.ClearModel();
    pista.ClearModel();
    pista_espiral.ClearModel();
    poste_luz.ClearModel();
    soporte_faro_lateral.ClearModel();
    soportes_y_senalizacion.ClearModel();
    torre_comunicaciones.ClearModel();
    torre_control.ClearModel();
    for (auto& modelo : montanas)
        modelo.ClearModel();
    roca_periferica.ClearModel();
    nave.ClearModel();
    delete fuego;
    delete malla_fuego;
    delete malla_halos;
    for (auto& modelo : asteroides)
        modelo.ClearModel();
    dron_explorador.ClearModel();
    delete textura_fuego_asteroide;
    delete textura_humo;
    delete textura_aspas;
    delete malla_estela_asteroide;
    delete malla_humo_aereo;
    delete malla_aspas_dron;
    for (auto& modelo : decorativos)
        modelo.ClearModel();
    for (auto& modelo : rivales)
        modelo.ClearModel();
    suelo.ClearModel();
}

//>>>>>>>>>>>>>>>>>>>>>> Geometria auxiliar: orientacion de vuelo >>>>>>>>>>>>>>>>>>>
glm::mat4 EscenaPlanetaNaranja::MarcoDeVuelo(const glm::vec3& posicion, const glm::vec3& tangente)
{
    glm::vec3 x = glm::normalize(tangente);
    glm::vec3 z = glm::normalize(glm::cross(x, glm::vec3(0, 1, 0)));
    glm::vec3 y = glm::normalize(glm::cross(z, x));
    return glm::mat4(glm::vec4(x, 0), glm::vec4(y, 0), glm::vec4(z, 0), glm::vec4(posicion, 1));
}

//>>>>>>>>>>>>>>>>>>>>>> Curva del asteroide y su marco padre >>>>>>>>>>>>>>>>>>>
glm::mat4 EscenaPlanetaNaranja::MatrizAsteroide(float tiempo, int indice)
{
    float fase = glm::two_pi<float>() * (tiempo / RecorridoAereo::periodo_asteroides + indice / 3.0f);
    return MarcoDeVuelo(RecorridoAereo::Elipse(fase) + glm::vec3(0, altura_suelo, 0),
                        RecorridoAereo::TangenteElipse(fase));
}

//>>>>>>>>>>>>>>>>>>>>>> Visibilidad en el limite del cielo >>>>>>>>>>>>>>>>>>>
float EscenaPlanetaNaranja::VisibilidadAsteroide(const glm::mat4& matriz)
{
    // Un cubemap es infinito: el limite visual del cielo representa su entrada y salida.
    // La elipse continua completa fuera de ese limite, sin reinicios ni teletransportes.
    float u = glm::clamp((std::abs(matriz[3].x) - 1650.0f) / 250.0f, 0.0f, 1.0f);
    return 1.0f - u * u * (3.0f - 2.0f * u);
}

//>>>>>>>>>>>>>>>>>>>>>> Configuracion de los cinco recorridos de dron >>>>>>>>>>>>>>>>>>>
EscenaPlanetaNaranja::ConfiguracionDron EscenaPlanetaNaranja::ConfigurarDron(int indice)
{
    glm::vec3 eje = glm::normalize(posicion_torre_control - posicion_torre_blanca);
    float giro_eje = std::atan2(-eje.z, eje.x);
    glm::vec3 centro = (posicion_torre_blanca + posicion_torre_control) * 0.5f;
    float largo = 205.0f, ancho = 155.0f, giro = giro_eje;
    if (indice == 1)
    {
        centro = posicion_torre_blanca - glm::vec3(0, 0, 105);
        largo = 210;
        giro = -glm::half_pi<float>();
    }
    if (indice == 2)
    {
        centro = posicion_torre_control + glm::vec3(0, 0, 105);
        largo = 210;
        giro = glm::half_pi<float>();
    }
    if (indice == 3)
    {
        centro = posicion_torre_blanca - eje * 105.0f;
        largo = 210;
        giro = giro_eje;
    }
    if (indice == 4)
    {
        centro = posicion_torre_control + eje * 105.0f;
        largo = 210;
        giro = giro_eje + glm::pi<float>();
    }
    centro.y = altura_suelo + 270.0f + indice * 35.0f;
    return {
        centro, largo, ancho, giro, indice % 2 == 0 ? 1.0f : -1.0f, glm::two_pi<float>() * indice / 5.0f};
}

//>>>>>>>>>>>>>>>>>>>>>> Curva del dron y su marco padre >>>>>>>>>>>>>>>>>>>
glm::mat4 EscenaPlanetaNaranja::MatrizDron(float tiempo, int indice)
{
    ConfiguracionDron configuracion = ConfigurarDron(indice);
    float fase = configuracion.sentido * glm::two_pi<float>() * tiempo / RecorridoAereo::periodo_drones +
                 configuracion.fase;
    glm::mat4 giro = glm::rotate(glm::mat4(1), configuracion.giro, glm::vec3(0, 1, 0));
    glm::vec3 punto =
        configuracion.centro +
        glm::vec3(giro *
                  glm::vec4(RecorridoAereo::Ocho(fase, configuracion.largo, configuracion.ancho), 0));
    glm::vec3 tangente = glm::vec3(
        giro * glm::vec4(RecorridoAereo::TangenteOcho(fase, configuracion.largo, configuracion.ancho) *
                             configuracion.sentido,
                         0));
    // El dron mira a +X local: su morro sigue la tangente de la curva.
    return glm::scale(MarcoDeVuelo(punto, tangente), glm::vec3(escala_dron));
}

//>>>>>>>>>>>>>>>>>>>>>> Consulta de la trayectoria por distancia >>>>>>>>>>>>>>>>>>>
glm::vec3 EscenaPlanetaNaranja::PuntoPista(float distancia) const
{
    return RecorridoNaranja::PuntoCentro(distancia, distancias);
}

//>>>>>>>>>>>>>>>>>>>>>> Geometria auxiliar: lectura de triangulos >>>>>>>>>>>>>>>>>>>
std::vector<EscenaPlanetaNaranja::TrianguloPista> EscenaPlanetaNaranja::LeerSeccionesPista(const char* ruta)
{
    std::vector<TrianguloPista> triangulos;
    std::ifstream archivo(ruta);
    std::vector<glm::vec3> vertices;
    std::string linea;
    while (std::getline(archivo, linea))
    {
        if (linea.compare(0, 2, "v ") == 0)
        {
            std::istringstream entrada(linea.substr(2));
            glm::vec3 v;
            entrada >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (linea.compare(0, 2, "f ") == 0)
        {
            std::istringstream entrada(linea.substr(2));
            std::vector<int> indices;
            std::string esquina;
            while (entrada >> esquina)
                indices.push_back(std::stoi(esquina) - 1);
            for (size_t i = 1; i + 1 < indices.size(); ++i)
                triangulos.push_back(
                    {vertices[indices[0]], vertices[indices[i]], vertices[indices[i + 1]]});
        }
    }
    return triangulos;
}

//>>>>>>>>>>>>>>>>>>>>>> Geometria auxiliar: cruces de faros con la pista >>>>>>>>>>>>>>>>>>>
bool EscenaPlanetaNaranja::FaroInvadeTableroSuperior(const glm::vec3& base,
                                                     const glm::vec3& hacia_dentro) const
{
    for (float avance : {0.0f, 3.5f, 7.2f})
    {
        glm::vec3 p = base + hacia_dentro * avance;
        for (const auto& triangulo : secciones_espiral)
        {
            const glm::vec3& a = triangulo[0];
            const glm::vec3& b = triangulo[1];
            const glm::vec3& c = triangulo[2];
            if ((p.x < a.x && p.x < b.x && p.x < c.x) || (p.x > a.x && p.x > b.x && p.x > c.x) ||
                (p.z < a.z && p.z < b.z && p.z < c.z) || (p.z > a.z && p.z > b.z && p.z > c.z))
                continue;
            glm::vec3 d = b - a, e = c - a;
            float determinante = d.x * e.z - d.z * e.x;
            if (std::abs(determinante) < 1e-6f)
                continue;
            glm::vec3 q = p - a;
            float u = (q.x * e.z - q.z * e.x) / determinante;
            float v = (d.x * q.z - d.z * q.x) / determinante;
            if (u < 0 || v < 0 || u + v > 1)
                continue;
            float altura = a.y + d.y * u + e.y * v;
            if (altura > base.y + 2.0f && altura < base.y + 21.0f)
                return true;
        }
    }
    return false;
}

//>>>>>>>>>>>>>>>>>>>>>> Geometria auxiliar: bordes de una seccion >>>>>>>>>>>>>>>>>>>
glm::vec2 EscenaPlanetaNaranja::BordesEnSeccion(const glm::vec3& centro, const glm::vec3& adelante,
                                                const glm::vec3& lateral) const
{
    glm::vec2 limites(1.0e6f, -1.0e6f);
    for (const auto& triangulo : secciones_pista)
        for (int k = 0; k < 3; ++k)
        {
            glm::vec3 a = triangulo[k], b = triangulo[(k + 1) % 3];
            float da = glm::dot(a - centro, adelante), db = glm::dot(b - centro, adelante);
            if (da * db > 0.0f || std::abs(da - db) < 1e-5f)
                continue;
            glm::vec3 corte = glm::mix(a, b, da / (da - db));
            float distancia = glm::dot(corte - centro, lateral);
            if (std::abs(corte.y - centro.y) > 4.0f || std::abs(distancia) > 85.0f)
                continue;
            limites.x = std::min(limites.x, distancia);
            limites.y = std::max(limites.y, distancia);
        }
    return limites.x <= limites.y ? limites : glm::vec2(-60.0f, 60.0f);
}

//>>>>>>>>>>>>>>>>>>>>>> Geometria auxiliar: separacion del circuito >>>>>>>>>>>>>>>>>>>
float EscenaPlanetaNaranja::DistanciaCircuitoPlanta(const glm::vec3& punto)
{
    float menor = 1.0e20f;
    glm::vec2 p(punto.x, punto.z);
    for (int i = 1; i < RecorridoNaranja::cantidad; ++i)
    {
        glm::vec2 a(RecorridoNaranja::puntos[i - 1].x, RecorridoNaranja::puntos[i - 1].z);
        glm::vec2 b(RecorridoNaranja::puntos[i].x, RecorridoNaranja::puntos[i].z);
        glm::vec2 d = b - a;
        float t = glm::clamp(glm::dot(p - a, d) / std::max(glm::dot(d, d), 1e-4f), 0.0f, 1.0f);
        glm::vec2 separacion = p - a - t * d;
        menor = std::min(menor, glm::dot(separacion, separacion));
    }
    return std::sqrt(menor);
}

//>>>>>>>>>>>>>>>>>>>>>> Geometria auxiliar: peralte del tablero >>>>>>>>>>>>>>>>>>>
glm::vec3 EscenaPlanetaNaranja::PuntoTablero(float tiempo, float carril)
{
    return RecorridoEspiral::PuntoCarril(tiempo, carril);
}

namespace
{
// Una vuelta completa y un cruce diametral alternan el extremo de salida.
glm::vec2 PuntoRocky(float tiempo)
{
    const float radio = 480.0f;
    const float vuelta = glm::two_pi<float>()*radio;
    const float recorrido = vuelta+2.0f*radio;
    const float ciclo = std::fmod(tiempo*5.5f,2.0f*recorrido);
    const float lado = ciclo<recorrido ? 1.0f : -1.0f;
    const float d = std::fmod(ciclo,recorrido);
    if (d<vuelta)
        return glm::vec2(-90,0)+lado*radio*glm::vec2(std::cos(d/radio),std::sin(d/radio));
    return glm::vec2(-90+lado*(radio-(d-vuelta)),0);
}
}

bool EscenaPlanetaNaranja::ObjetivoCamara(int modo, glm::vec3& objetivo, glm::vec3& rumbo,
                                 float& distancia, float& altura) const
{
    if (modo == 5 && caminante_cargado)
    {
        const glm::vec2 p = PuntoRocky(tiempo_rocky);
        const glm::vec2 d = PuntoRocky(tiempo_rocky+.08f)-PuntoRocky(tiempo_rocky-.08f);
        objetivo = {p.x,altura_suelo+AlturaSuelo(p.x,p.y)+3.0f,p.y};
        rumbo = {d.x,0,d.y};
        distancia = 22.0f;
        altura = altura_suelo+13.0f;
        return true;
    }
    if (modo != 2) return false;
    objetivo = glm::vec3(matriz_nave_espiral[3]);
    rumbo = -glm::vec3(matriz_nave_espiral[0]);
    distancia = 22.0f;
    altura = objetivo.y + 10.0f;
    return true;
}

glm::mat4 EscenaPlanetaNaranja::MarcoFaroRival(bool espiral) const
{
    const int indice = espiral ? 0 : 11;
    return glm::rotate(matrices_rivales[indice],
        -glm::radians(giros_rivales[configuraciones_rivales[indice].modelo]),glm::vec3(0,1,0));
}

//>>>>>>>>>>>>>>>>>>>>>> Rocky animado y jerarquias de los domos >>>>>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::CargarRocky()
{
    const std::string ruta = "assets/planeta_naranja/modelos/";
    caminante_cargado = rocky_caminante.Cargar(ruta+"rocky_caminando.fbx",
        "assets/planeta_naranja/texturas/rocky_caminando.png",true);
    protos_cargado = protos.Cargar(ruta+"protos.fbx","",true);
    if (protos_cargado) protos.QuitarAvance(0,"mixamorig:Hips");
    if (caminante_cargado)
    {
        rocky_caminante.QuitarAvance(0,"Hips");
        shader_rocky.CreateFromFiles("shaders/shader_esqueleto.vert", "shaders/shader_light.frag");
    }
    rocky_saludo[0].LoadModel(ruta+"rocky_torso.obj",false,false);
    rocky_saludo[1].LoadModel(ruta+"rocky_brazo_derecho.obj",false,false);
    rocky_saludo[2].LoadModel(ruta+"rocky_mano_derecha.obj",false,false);
    rocky_saludo[3].LoadModel(ruta+"rocky_brazo_izquierdo.obj",false,false);
    rocky_saludo[4].LoadModel(ruta+"rocky_mano_izquierda.obj",false,false);
    rocky_saludo[5].LoadModel(ruta+"rocky_pata_izquierda_superior.obj",false,false);
    rocky_saludo[6].LoadModel(ruta+"rocky_pata_izquierda_inferior.obj",false,false);
    rocky_saludo[7].LoadModel(ruta+"rocky_pata_trasera_superior.obj",false,false);
    rocky_saludo[8].LoadModel(ruta+"rocky_pata_trasera_inferior.obj",false,false);
    rocky_saludo[9].LoadModel(ruta+"rocky_pata_derecha_superior.obj",false,false);
    rocky_saludo[10].LoadModel(ruta+"rocky_pata_derecha_inferior.obj",false,false);
}

void EscenaPlanetaNaranja::DibujarRocky(const ContextoRender& ctx)
{
    auto articular = [](const glm::mat4& padre,glm::vec3 pivote,float angulo,glm::vec3 eje) {
        return glm::translate(glm::rotate(glm::translate(padre,pivote),angulo,eje),-pivote);
    };
    auto pieza = [&](int indice,const glm::mat4& matriz) {
        glUniformMatrix4fv(ctx.u_model,1,GL_FALSE,glm::value_ptr(matriz));
        rocky_saludo[indice].RenderModel(true,ctx.u_specular_intensity,ctx.u_shininess);
    };
    glUniform3f(ctx.u_color,1,1,1);
    const glm::vec2 domos[] = {{-225,-75},{395,425}};
    for (int i = 0; i < 2; ++i)
    {
        const auto& p = domos[i];
        glm::mat4 raiz = glm::translate(glm::mat4(1),glm::vec3(p.x,altura_suelo+AlturaSuelo(p.x,p.y),p.y));
        raiz = glm::scale(glm::rotate(raiz,i*.8f,glm::vec3(0,1,0)),glm::vec3(14));
        const float fase = std::fmod(tiempo_rocky+i*2.5f,8.0f);
        const float alzar = RecorridoEspiral::Suavizar((fase-1.0f)/.9f) *
                            RecorridoEspiral::Suavizar((5.8f-fase)/.9f);
        const glm::mat4 brazo = articular(raiz,{-.148f,.589f,.167f},-1.65f*alzar,{0,0,1});
        const glm::mat4 mano = articular(brazo,{-.322f,.456f,.250f},.28f*alzar*std::sin(fase*9.0f),{0,0,1});
        pieza(0,raiz); pieza(1,brazo); pieza(2,mano);
        const glm::mat4 brazo_alto = articular(raiz,{.148f,.589f,.167f},
            .27f*alzar + .19f*alzar*std::sin(fase*9.0f),{0,0,1});
        pieza(3,brazo_alto);
        pieza(4,articular(brazo_alto,{.365f,.670f,.319f},.26f*alzar*std::sin(fase*9.0f),{0,1,0}));
        pieza(5,raiz); pieza(6,raiz);
        pieza(7,raiz); pieza(8,raiz);
        pieza(9,raiz); pieza(10,raiz);
    }
    if (!caminante_cargado) return;
    GLint anterior = 0, programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM,&anterior);
    shader_rocky.UseShader();
    glGetIntegerv(GL_CURRENT_PROGRAM,&programa);
    shader_rocky.SetDirectionalLight(&luz_sol);
    shader_rocky.SetPointLights(nullptr,0);
    shader_rocky.SetSpotLights(nullptr,0);
    glUniformMatrix4fv(shader_rocky.GetViewLocation(),1,GL_FALSE,glm::value_ptr(ctx.vista));
    glUniformMatrix4fv(shader_rocky.GetProjectionLocation(),1,GL_FALSE,glm::value_ptr(ctx.proyeccion));
    glUniform3fv(shader_rocky.GetEyePositionLocation(),1,glm::value_ptr(ctx.pos_camara));
    glUniform3f(shader_rocky.getColorLocation(),1,1,1);
    glUniform1f(shader_rocky.GetSpecularIntensityLocation(),.16f);
    glUniform1f(shader_rocky.GetShininessLocation(),16);
    auto caminar = [&](ModeloAnimado& actor,glm::vec2 p,glm::vec2 d,float fase,float escala) {
        glm::mat4 m = glm::translate(glm::mat4(1),glm::vec3(p.x,
            altura_suelo+AlturaSuelo(p.x,p.y)+(std::abs(p.y)<1 ? .55f : 0),p.y));
        m = glm::scale(glm::rotate(m,std::atan2(d.x,d.y),glm::vec3(0,1,0)),glm::vec3(escala)) *
            actor.Normalizacion();
        glUniformMatrix4fv(shader_rocky.GetModelLocation(),1,GL_FALSE,glm::value_ptr(m));
        actor.CalcularPose(0,fase);
        actor.Dibujar(GLuint(programa));
    };
    caminar(rocky_caminante,PuntoRocky(tiempo_rocky),PuntoRocky(tiempo_rocky+.08f)-PuntoRocky(tiempo_rocky-.08f),
            tiempo_rocky,10.0f);
    const glm::vec2 zonas[] = {{340,212},{340,295},{-335,-182},{345,-225},{-350,220},{-470,235},{140,-510},{440,-340}};
    for (int i = 0; i < 8; ++i)
    {
        const float a = tiempo_rocky*.45f+i*1.3f;
        const glm::vec3 ocho = RecorridoAereo::Ocho(a,6,4);
        const glm::vec3 tangente = RecorridoAereo::TangenteOcho(a,6,4);
        caminar(rocky_caminante,zonas[i]+glm::vec2(ocho.x,ocho.z),{tangente.x,tangente.z},tiempo_rocky+i*.23f,8.5f);
    }
    if (protos_cargado)
    {
        const glm::vec2 zonas_protos[] = {{435,340},{470,385},{530,305},{320,470},
                                         {-410,155},{-440,-220},{205,-535}};
        for (int i = 0; i < 7; ++i)
        {
            const float a = tiempo_rocky*.42f+i;
            const glm::vec3 p = RecorridoAereo::Ocho(a,5,3);
            const glm::vec3 d = RecorridoAereo::TangenteOcho(a,5,3);
            caminar(protos,zonas_protos[i]+glm::vec2(p.x,p.z),{d.x,d.z},tiempo_rocky+i*.17f,7);
        }
    }
    glUseProgram(anterior);
}
