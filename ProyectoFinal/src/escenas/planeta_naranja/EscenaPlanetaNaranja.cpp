//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y cabeceras >>>>>>>>>>>>>>>>>>>>>
#include "EscenaPlanetaNaranja.h"
#include "RecorridoNaranja.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Solo configuracion inmutable compartida; el estado pertenece a la escena.
namespace
{
const glm::vec3 posicion_torre_blanca(-150, 0, -90);
const glm::vec3 posicion_torre_control(-100, 0, 90);
const glm::vec3 pivotes_rotores[2] = {{-0.099f, 0.495f, -0.407f}, {-0.099f, 0.495f, 0.407f}};
} // namespace

//>>>>>>>>>>>>>>>>>>>>>> Inicializacion: orden de los recursos >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::Inicializar()
{
    CargarSkybox("assets/planeta_naranja/texturas/", ".jpg", "cielo_");
    camara = Camera(glm::vec3(110, altura_suelo + 1.7f, 55), glm::vec3(0, 1, 0), -120, 0, 35, 0.3f);
    CargarModelos();
    CargarTexturas();
    CrearFuego();
    CrearMallasAereas();
    CrearRecorrido();
    CrearPaisajePeriferico();
    CrearFaros();

    // Estado inicial de ambas naves, asteroides, rotores y emisores.
    avance_nave = tiempo_espiral = 0.0f;
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
                float angulo = orientacion * (6.283185307f / 16.0f);
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
}

//>>>>>>>>>>>>>>>>>>>>>> Animacion de la escena >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::Actualizar(float dt, Window& ventana)
{
    motor1 = ventana.getMotor1Encendido();
    motor2 = ventana.getMotor2Encendido();
    if (ventana.getVistaGeneralSolicitada())
        camara = Camera(glm::vec3(700.0f, altura_suelo + 700.0f, 1100.0f), glm::vec3(0, 1, 0), -126.0f,
                        -25.0f, 150.0f, 0.3f);
    ActualizarNaves(dt);
    ActualizarObjetosAereos(dt);
}

//>>>>>>>>>>>>>>>>>>>>>> Jerarquias completas: pistas -> naves -> motores >>>>>>>>>>>>>>>>>>>
void EscenaPlanetaNaranja::ActualizarNaves(float dt)
{
    // Padre 1: pista inferior -> trayectoria -> orientacion y escala de la nave.

    // Distancia real por segundo; conserva el sobrante al pasar de segmento.
    avance_nave = std::fmod(avance_nave + (12.0f / escala_pista) * dt, longitud_circuito);
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

    tiempo_espiral = std::fmod(tiempo_espiral + dt, RecorridoEspiral::tiempo_total);
    RecorridoEspiral::Estado estado = RecorridoEspiral::Evaluar(tiempo_espiral);
    matriz_nave_espiral = glm::scale(
        glm::mat4(glm::vec4(-estado.adelante, 0), glm::vec4(estado.arriba, 0),
                  glm::vec4(-estado.lateral, 0),
                  glm::vec4(estado.posicion + estado.arriba * 1.2f + glm::vec3(0, altura_suelo, 0), 1)),
        glm::vec3(0.198f));

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
    glUniform3f(ctx.u_color, 1.40f, 0.82f, 0.52f);
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
            glm::vec3(3.3f, 4.6f, 3.3f))));
    torre_comunicaciones.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);

    glUniformMatrix4fv(
        ctx.u_model, 1, GL_FALSE,
        glm::value_ptr(
            glm::scale(glm::translate(glm::mat4(1), posicion_torre_control +
                                                        glm::vec3(0, altura_suelo + 410.291962f * 0.8f, 0)),
                       glm::vec3(0.8f))));
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

    DibujarNaves(ctx);
    DibujarObjetosAereos(ctx);
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
    glm::mat4 vista = camara.calculateViewMatrix();
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
glm::vec3 EscenaPlanetaNaranja::ObtenerPosicionLuzPoste(int indice, const glm::vec3& ojo) const
{
    // El presupuesto de luces ilumina las luminarias cercanas a la camara.
    std::vector<glm::vec3> posiciones;
    for (const auto& matriz : matrices_postes)
        posiciones.push_back(glm::vec3(matriz * glm::vec4(0, 1.37f, -0.36f, 1)));
    std::sort(posiciones.begin(), posiciones.end(), [&ojo](const glm::vec3& a, const glm::vec3& b) {
        return glm::dot(a - ojo, a - ojo) < glm::dot(b - ojo, b - ojo);
    });
    return posiciones[indice % posiciones.size()];
}

//>>>>>>>>>>>>>>>>>>>>>> Liberacion de recursos >>>>>>>>>>>>>>>>>>>
EscenaPlanetaNaranja::~EscenaPlanetaNaranja()
{
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
    for (auto& modelo : asteroides)
        modelo.ClearModel();
    dron_explorador.ClearModel();
    delete textura_fuego_asteroide;
    delete textura_humo;
    delete textura_aspas;
    delete malla_estela_asteroide;
    delete malla_humo_aereo;
    delete malla_aspas_dron;
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
    // El frente del modelo apunta a -X, igual que la nave de carreras.
    return glm::scale(glm::rotate(MarcoDeVuelo(punto, tangente), glm::pi<float>(), glm::vec3(0, 1, 0)),
                      glm::vec3(escala_dron));
}

//>>>>>>>>>>>>>>>>>>>>>> Consulta de la trayectoria por distancia >>>>>>>>>>>>>>>>>>>
glm::vec3 EscenaPlanetaNaranja::PuntoPista(float distancia) const
{
    float s = std::fmod(distancia, longitud_circuito);
    if (s < 0.0f)
        s += longitud_circuito;
    auto siguiente = std::upper_bound(distancias.begin(), distancias.end(), s);
    size_t i = static_cast<size_t>(siguiente - distancias.begin() - 1);
    float u = (s - distancias[i]) / (distancias[i + 1] - distancias[i]);
    return glm::mix(RecorridoNaranja::puntos[i], RecorridoNaranja::puntos[i + 1], u);
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
            if (std::abs(determinante) < 0.000001f)
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
            if (da * db > 0.0f || std::abs(da - db) < 0.00001f)
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
        float t = glm::clamp(glm::dot(p - a, d) / std::max(glm::dot(d, d), 0.0001f), 0.0f, 1.0f);
        glm::vec2 separacion = p - a - t * d;
        menor = std::min(menor, glm::dot(separacion, separacion));
    }
    return std::sqrt(menor);
}

//>>>>>>>>>>>>>>>>>>>>>> Geometria auxiliar: peralte del tablero >>>>>>>>>>>>>>>>>>>
glm::vec3 EscenaPlanetaNaranja::PuntoTablero(float tiempo, float carril)
{
    if (tiempo <= 3.0f || tiempo >= 18.0f)
        return RecorridoEspiral::PuntoCarril(tiempo, carril);
    float peralte;
    glm::vec3 centro = RecorridoEspiral::Punto(tiempo, &peralte);
    glm::vec3 anterior = RecorridoEspiral::Lateral(tiempo, peralte);
    // Borde bajo horizontal al inicio y union C2 con la subida, sin hundimiento.
    const float pendiente = 135.0f / 140.0f;
    const float subida = (80.0f + pendiente * 14.0f) - 11.0f - 80.0f;
    const float duracion = 2.0f * subida / pendiente;
    float u = glm::clamp((tiempo - 18.0f + duracion) / duracion, 0.0f, 1.0f);
    float borde = 80.0f + pendiente * duracion * (u * u * u - 0.5f * u * u * u * u);
    float angulo = std::asin(glm::clamp((centro.y - anterior.y * 5.5f - borde) / 27.5f, -1.0f, 1.0f));
    glm::vec3 nuevo =
        RecorridoEspiral::Lateral(tiempo, 0.0f) * std::cos(angulo) - glm::vec3(0, 1, 0) * std::sin(angulo);
    return centro - anterior * 5.5f + nuevo * (carril + 5.5f);
}
