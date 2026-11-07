//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y cabeceras >>>>>>>>>>>>>>>>>>>>>
#include "EscenaEspacio.h"
#include "planeta_naranja/RecorridoNaranja.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <fstream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/constants.hpp>
//>>>>>>>>>>>>>>>>>>>>>> Configuracion y funciones geometricas >>>>>>>
namespace
{
const float pi = glm::pi<float>();
// Solo datos inmutables que comparten la carga, la animacion y el dibujo.
const glm::vec3 origen_estacion(254.778f, 202.409f, 1036.046f);
struct ConfiguracionPlaneta
{
    const char* modelo;
    glm::vec3 centro;
    float radio, inclinacion, periodo_rotacion;
    glm::vec3 color_atmosfera;
};
const ConfiguracionPlaneta configuraciones_planetas[] = {
    {"planeta_azul", {0, -495, -13480}, 2200, -8, 180, {.15f, .45f, 1}},
    {"planeta_naranja", {13500, 355, 7020}, 2500, 14, 160, {1, .3f, .08f}},
    {"planeta_morado", {-13200, 605, 6520}, 2350, -12, 210, {.55f, .18f, 1}}};
float Fase(double tiempo, float periodo)
{
    return float(std::fmod(tiempo / periodo, 1.0)) * 2 * pi;
}
glm::mat4 Trasladar(glm::vec3 p)
{
    return glm::translate(glm::mat4(1), p);
}
glm::mat4 Girar(float a, glm::vec3 eje)
{
    return glm::rotate(glm::mat4(1), a, eje);
}
glm::mat4 Escalar(float s)
{
    return glm::scale(glm::mat4(1), glm::vec3(s));
}
//>>>>>>>>>>>>>>>>>>>>>> Circuito interplanetario >>>>>>>>>>>>>>>>>>>>
glm::vec3 PuntoAurora(float fase)
{
    const float giro_muelle = 4.0f * pi / 3.0f;
    const glm::vec3 muelle = origen_estacion + 22.0f *
        glm::vec3(Girar(-giro_muelle, glm::vec3(0, 1, 0)) * glm::vec4(-3.5f, -.048f, 49.3f, 1));
    const glm::vec3 salida(muelle.x, 420.0f, muelle.z);
    if (fase < 6.0f)
        return glm::mix(muelle, salida, RecorridoEspiral::Suavizar((fase - 4.0f) / 2.0f));

    glm::vec3 anterior = salida;
    glm::vec3 velocidad_anterior(0);
    fase -= 6.0f;
    for (int i = 0; i < 3; ++i)
    {
        const auto& planeta = configuraciones_planetas[i];
        const glm::vec3 centro(planeta.centro.x, planeta.centro.y, planeta.centro.z);
        const float radio = planeta.radio + 1350.0f;
        const glm::vec3 radial = glm::normalize(glm::vec3(anterior.x-centro.x,0,anterior.z-centro.z));
        const glm::vec3 entrada = centro + radial * radio;
        const glm::vec3 tangente(-radial.z, 0, radial.x);
        const glm::vec3 velocidad_vuelta = tangente * (radio * 2.0f*pi / 20.0f);
        if (fase < 12.0f)
            return RecorridoEspiral::Hermite(anterior, entrada,
                i == 0 ? entrada - anterior : velocidad_anterior * 12.0f,
                velocidad_vuelta * 12.0f, fase / 12.0f);
        if (fase < 32.0f)
        {
            const float angulo = (fase - 12.0f) * (2.0f * pi / 20.0f);
            return centro + radio * (radial * std::cos(angulo) + tangente * std::sin(angulo));
        }
        anterior = entrada;
        velocidad_anterior = velocidad_vuelta;
        fase -= 32.0f;
    }
    if (fase < 22.0f)
        return RecorridoEspiral::Hermite(anterior, salida, velocidad_anterior * 22.0f,
                                        glm::vec3(0), fase / 22.0f);
    return glm::mix(salida, muelle, RecorridoEspiral::Suavizar((fase - 22.0f) / 2.0f));
}

glm::mat4 MatrizAurora(float fase)
{
    const glm::vec3 posicion = PuntoAurora(fase);
    const glm::vec3 avance = PuntoAurora(fase + .025f) - PuntoAurora(fase - .025f);
    const float giro_muelle = -4.0f * pi / 3.0f;
    float giro = giro_muelle;
    if (fase >= 6.0f && fase <= 124.0f)
        giro = std::atan2(avance.x, avance.z);
    else if (fase > 4.0f && fase < 6.0f)
    {
        const glm::vec3 rumbo = PuntoAurora(6.1f) - PuntoAurora(6.0f);
        giro += std::remainder(std::atan2(rumbo.x, rumbo.z) - giro, 2.0f*pi) *
                RecorridoEspiral::Suavizar((fase-4.0f)/2.0f);
    }
    else if (fase > 124.0f)
    {
        const glm::vec3 rumbo = PuntoAurora(124.0f) - PuntoAurora(123.9f);
        giro += std::remainder(std::atan2(rumbo.x, rumbo.z) - giro, 2.0f*pi) *
                (1.0f-RecorridoEspiral::Suavizar((fase-124.0f)/2.0f));
    }
    return Trasladar(posicion) * Girar(giro, glm::vec3(0, 1, 0)) * Escalar(25.3f);
}

//>>>>>>>>>>>>>>>>>>>>>> Cuadrantes y keyframes de las naves >>>>>>>>>>>>>>>>>>
struct ConfiguracionNave
{
    const char* modelo;
    int cuadrante;
    float escala;
    float giro_estacionado = 90.0f;
    float ajuste_lateral = 0.0f;
    float ajuste_longitudinal = 0.0f;
    float giro_modelo = 0.0f;
};
const glm::vec3 cuadrantes_aterrizaje[] = {
    {-4, -.048f, 41}, {4, -.053f, 41}, {-4.5f, -.048f, 48}, {4.5f, -.048f, 48}};
const float alturas_despegue[] = {12, 18, 28, 36};
const ConfiguracionNave configuraciones_naves[] = {{"nave_grace", 0, 0.480f, 180.0f, 0.0f, 0.0f, -90.0f},
                                                   {"nave_rocky", 1, 0.480f, 180.0f, 0.0f, 0.0f, -90.0f},
                                                   {"nave_figfighter", 2, 0.960f},
                                                   {"nave_talros", 3, 0.960f},
                                                   {"nave_02", 0, 0.950f},
                                                   {"nave_03", 1, 0.950f, 180.0f, 0.0f, 0.0f, -90.0f},
                                                   {"nave_04", 2, 0.950f},
                                                   {"nave_07", 3, 0.950f},
                                                   {"nave_08", 0, 0.950f},
                                                   {"nave_10", 1, 0.950f},
                                                   {"nave_11", 2, 0.950f},
                                                   {"nave_12", 3, 0.950f},
                                                   {"nave_14", 0, 0.950f, 180.0f, 0.0f, 0.0f, -90.0f},
                                                   {"nave_16", 1, 0.950f},
                                                   {"nave_aurora", 2, 1.150f, 0.0f, 1.0f, 1.300f}};
struct FrameNave
{
    float avance, altura, giro;
};
const FrameNave keyframes_naves[] = {{0, 0, 0},           {0, 1, 0}, {0, 1, 1}, {1.0f / 3.0f, 1, 1},
                                     {2.0f / 3.0f, 1, 1}, {1, 1, 1}, {1, 1, 0}, {1, 0, 0}};
const float duracion_keyframe = 6.0f;
const float pasos_keyframe = 90.0f;

FrameNave InterpolarNave(int indice, float pasos)
{
    const FrameNave& inicio = keyframes_naves[indice];
    const FrameNave& fin = keyframes_naves[indice + 1];
    return {inicio.avance + (fin.avance - inicio.avance) / pasos_keyframe * pasos,
            inicio.altura + (fin.altura - inicio.altura) / pasos_keyframe * pasos,
            inicio.giro + (fin.giro - inicio.giro) / pasos_keyframe * pasos};
}

// Arco exterior abierto: sale radialmente y llega radialmente a la siguiente plataforma.
glm::vec3 PuntoVueloNave(float avance, float altura, int nave, int plataforma)
{
    int lugar = configuraciones_naves[nave].cuadrante;
    glm::vec3 cuadrante =
        cuadrantes_aterrizaje[lugar] + glm::vec3(configuraciones_naves[nave].ajuste_lateral, 0,
                                                 configuraciones_naves[nave].ajuste_longitudinal);
    float angulo = std::atan2(cuadrante.z, cuadrante.x) +
                   2.0f * pi / 3.0f * (plataforma + RecorridoEspiral::Suavizar(avance));
    float radio =
        glm::length(glm::vec2(cuadrante.x, cuadrante.z)) + (125.0f + 20.0f * lugar) * std::sin(pi * avance);
    glm::vec3 punto = CurvasOrbitales::ElipsePlana(angulo, radio, radio);
    punto.y = cuadrante.y + alturas_despegue[lugar] * RecorridoEspiral::Suavizar(altura) +
              8.0f * std::sin(pi * avance);
    return punto;
}

// Cada intervalo dura lo mismo; las distancias cortas producen despegues y aterrizajes lentos.
glm::mat4 MatrizVueloNave(double tiempo, int nave)
{
    float paso = float(std::fmod((tiempo + 31.5 * (nave / 4)) / duracion_keyframe, 21.0));
    int intervalo = int(paso);
    int cuadro = intervalo % 7;
    int plataforma = (intervalo / 7) % 3;
    FrameNave frame = InterpolarNave(cuadro, (paso - intervalo) * pasos_keyframe);
    glm::vec3 tangente = PuntoVueloNave(glm::min(frame.avance + .001f, 1.0f), 1, nave, plataforma) -
                         PuntoVueloNave(glm::max(frame.avance - .001f, 0.0f), 1, nave, plataforma);
    float giro = std::atan2(tangente.x, tangente.z);
    if (cuadro < 2 || cuadro >= 5)
    {
        float estacionado = -(plataforma + (cuadro >= 5 ? 1 : 0)) * 2.0f * pi / 3.0f +
                            glm::radians(configuraciones_naves[nave].giro_estacionado);
        giro = estacionado + std::remainder(giro - estacionado, 2.0f * pi) * frame.giro;
    }
    return Trasladar(PuntoVueloNave(frame.avance, frame.altura, nave, plataforma)) *
           Girar(giro + glm::radians(configuraciones_naves[nave].giro_modelo), glm::vec3(0, 1, 0)) *
           Escalar(configuraciones_naves[nave].escala);
}

MeshModel* CrearEsfera()
{
    std::vector<GLfloat> v;
    std::vector<unsigned> indices;
    for (int i = 0; i <= 48; ++i)
        for (int j = 0; j <= 96; ++j)
        {
            float p = pi * i / 48, a = 2 * pi * j / 96;
            float x = sin(p) * cos(a), y = cos(p), z = sin(p) * sin(a);
            v.insert(v.end(), {x, y, z, float(j) / 96, 1 - float(i) / 48, x, y, z});
        }
    for (int i = 0; i < 48; ++i)
        for (int j = 0; j < 96; ++j)
        {
            unsigned a = i * 97 + j, b = a + 97;
            indices.insert(indices.end(), {a, a + 1, b, a + 1, b + 1, b});
        }
    auto* m = new MeshModel;
    m->CreateMeshModel(v.data(), indices.data(), unsigned(v.size()), unsigned(indices.size()));
    return m;
}
MeshModel* CrearEstela()
{
    GLfloat v[] = {0, -.5f, 0, 0,   0,  0, 0,   1, -1, -.5f, 0, 0, 1,    0, 0,   1, -1, .5f, 0,  1, 1,    0,
                   0, 1,    0, .5f, 0,  1, 0,   0, 0,  1,    0, 0, -.5f, 0, 0,   0, 1,  0,   -1, 0, -.5f, 0,
                   1, 0,    1, 0,   -1, 0, .5f, 1, 1,  0,    1, 0, 0,    0, .5f, 1, 0,  0,   1,  0};
    unsigned idx[] = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
    auto* m = new MeshModel;
    m->CreateMeshModel(v, idx, 64, 12);
    return m;
}
} // namespace

//>>>>>>>>>>>>>>>>>>>>>> Inicializacion: orden de los recursos >>>>>>>>>>>>>>>>>>>
void EscenaEspacio::Inicializar()
{
    CargarSkybox("assets/espacio/texturas/", ".jpg", "cielo_");
    camara = Camera(origen_estacion + glm::vec3(2100, 1100, 2800), glm::vec3(0, 1, 0), -126.87f, -17.447f,
                    550, .3f);
    CargarModelos();
    CargarTexturas();
    esfera_atmosfera = CrearEsfera();
    malla_estela = CrearEstela();
    tiempo = 5.0;
    ActualizarMatrices();
}

//>>>>>>>>>>>>>>>>>>>>>> Carga de modelos >>>>>>>>>>>>>>>>>>>
void EscenaEspacio::CargarModelos()
{
    for (int i = 0; i < 3; ++i)
    {
        planetas[i].LoadModel(std::string("assets/espacio/modelos/") + configuraciones_planetas[i].modelo +
                                  ".obj",
                              false, false);
        asteroides[i].LoadModel(
            "assets/planeta_naranja/modelos/asteroide_0" + std::to_string(i + 1) + ".obj", false, false);
    }
    for (int i = 0; i < 2; ++i)
        cuerpos_satelites[i].LoadModel(
            "assets/espacio/modelos/satelite_0" + std::to_string(i + 1) + "_cuerpo.obj", false, false);
    estacion_xq6.LoadModel("assets/espacio/modelos/estacion_xq6.obj", false, false);
    cabina_estacion.LoadModel("assets/espacio/modelos/cabina_estacion.obj", false, false);
    campos_estacion.LoadModel("assets/espacio/modelos/campos_estacion.obj", false, false);
    for (int i = 0; i < 15; ++i)
        naves_estacion[i].LoadModel(std::string("assets/espacio/modelos/") +
                                        configuraciones_naves[i].modelo + ".obj",
                                    false, false);
    CargarPaneles();
    lunas[0].LoadModel("assets/espacio/modelos/luna_azul.obj", false, false);
    lunas[1].LoadModel("assets/espacio/modelos/luna_morada.obj", false, false);
}

//>>>>>>>>>>>>>>>>>>>>>> Carga de paneles: modelos, bisagras y ejes >>>>>>>>>>>>>>>>>>>
void EscenaEspacio::CargarPaneles()
{
    std::ifstream datos("assets/espacio/modelos/satelites_paneles.dat");
    int tipo, indice;
    glm::vec3 p, e;
    while (datos >> tipo >> indice >> p.x >> p.y >> p.z >> e.x >> e.y >> e.z)
    {
        if (tipo < 0 || tipo > 1 || indice < 0 || indice >= 8)
            continue;
        auto& panel = paneles[tipo][indice];
        panel.pivote = p;
        panel.eje = glm::normalize(e);
        panel.modelo.LoadModel("assets/espacio/modelos/satelite_0" + std::to_string(tipo + 1) + "_panel_0" +
                                   std::to_string(indice + 1) + ".obj",
                               false, false);
        cantidad_paneles[tipo] = std::max(cantidad_paneles[tipo], indice + 1);
    }
}

//>>>>>>>>>>>>>>>>>>>>>> Carga de texturas >>>>>>>>>>>>>>>>>>>
void EscenaEspacio::CargarTexturas()
{
    textura_brillos = Texture("assets/planeta_naranja/texturas/halo_faro.png");
    textura_brillos.LoadTextureA();

    textura_nubes = Texture("assets/espacio/texturas/nubes_azules.png");
    textura_nubes.LoadTextureA();
    textura_fuego = Texture("assets/planeta_naranja/texturas/fuego_asteroides.png");
    textura_fuego.LoadTextureA();
}

//>>>>>>>>>>>>>>>>>>>>>> Animacion: avance del tiempo >>>>>>>>>>>>>>>>>>>
void EscenaEspacio::Actualizar(float dt, Window&)
{
    tiempo += std::max(0.0f, dt);
    ActualizarMatrices();
}

//>>>>>>>>>>>>>>>>>>>>>> Jerarquias completas: planetas, orbitas y piezas hijas >>>>>>>>>>>>>>>>>>>
void EscenaEspacio::ActualizarMatrices()
{
    // Centro equidistante de los tres planetas -> estacion -> cabina articulada.
    matriz_aurora_exploradora = MatrizAurora(float(std::fmod(tiempo + 121.0, 126.0)));
    matriz_estacion = Trasladar(origen_estacion) * Escalar(22.0f);
    matriz_cabina = matriz_estacion * Trasladar(glm::vec3(0, 4.129f, 0)) *
                    Girar(Fase(tiempo, 120.0f), glm::vec3(0, 1, 0));
    for (int i = 0; i < 15; ++i)
        matrices_naves[i] = matriz_estacion * MatrizVueloNave(tiempo, i);
    for (int planeta = 0; planeta < 3; ++planeta)
    {
        float radio_planeta = configuraciones_planetas[planeta].radio;
        raices_planetas[planeta] =
            Trasladar(configuraciones_planetas[planeta].centro) *
            Girar(glm::radians(configuraciones_planetas[planeta].inclinacion), glm::vec3(0, 0, 1));
        // Sistema -> centro -> inclinacion -> giro diario -> escala del planeta.
        matrices_planetas[planeta] =
            raices_planetas[planeta] *
            Girar(Fase(tiempo, configuraciones_planetas[planeta].periodo_rotacion), glm::vec3(0, 1, 0)) *
            Escalar(radio_planeta);
        matrices_atmosferas[planeta] = raices_planetas[planeta] * Escalar(radio_planeta * 1.025f);
        glm::vec3 frente = glm::normalize(
            glm::vec3(glm::inverse(raices_planetas[planeta]) * glm::vec4(origen_estacion, 1)));
        glm::vec3 lateral_frente = glm::normalize(glm::cross(glm::vec3(0, 1, 0), frente)),
                  vertical_frente = glm::cross(frente, lateral_frente);
        for (int objeto = 0; objeto < 5; ++objeto)
        {
            float latitud = glm::radians(-50.0f + 25 * objeto),
                  angulo_orbital = Fase(tiempo, 50.0f + 5.0f * objeto + 3.0f * planeta) +
                                   2 * pi * (objeto / 5.0f + planeta * .11f);
            // Circulos de latitud: radio de seccion esferica mas altura orbital.
            float radio = radio_planeta * (cos(latitud) + .38f + .03f * objeto);
            // Planeta -> giro orbital -> latitud -> orientacion -> escala del satelite.
            matrices_satelites[planeta][objeto] =
                raices_planetas[planeta] * Girar(angulo_orbital, glm::vec3(0, 1, 0)) *
                Trasladar(glm::vec3(radio, radio_planeta * sin(latitud), 0)) *
                Girar(pi / 2, glm::vec3(0, 1, 0)) * Escalar(100);
            float sector = 2 * pi * objeto / 5;
            glm::vec3 normal_region = glm::normalize(
                frente + .55f * (lateral_frente * cos(sector) + vertical_frente * sin(sector)));
            glm::vec3 lateral_region = glm::normalize(glm::cross(glm::vec3(0, 1, 0), normal_region)),
                      vertical_region = glm::cross(normal_region, lateral_region);
            float fase = Fase(tiempo, 12.0f + 2.0f * objeto) + 2 * pi * (objeto / 5.0f + planeta * .13f);
            auto elipse = CurvasOrbitales::ElipsePlana(fase, .22f * radio_planeta, .10f * radio_planeta);
            auto tangente =
                CurvasOrbitales::TangenteElipse(fase, .22f * radio_planeta, .10f * radio_planeta);
            glm::vec3 direccion =
                glm::normalize(lateral_region * tangente.x + vertical_region * tangente.z);
            glm::mat4 marco(1);
            marco[0] = glm::vec4(direccion, 0);
            marco[1] = glm::vec4(normal_region, 0);
            marco[2] = glm::vec4(glm::cross(direccion, normal_region), 0);
            // El centro de cada ovalo esta sobre una region de la atmosfera, no en el planeta.
            // Planeta -> ovalo atmosferico -> marco de vuelo (padre de cuerpo y fuego).
            const glm::mat4 raiz_asteroide =
                raices_planetas[planeta] *
                Trasladar(normal_region * radio_planeta * (1.10f + .008f * objeto) +
                          lateral_region * elipse.x + vertical_region * elipse.z) *
                marco;
            // Raiz del asteroide -> cuerpo que gira; raiz -> fuego sin ese giro.
            matrices_asteroides[planeta][objeto] = raiz_asteroide *
                                                   Girar(float(std::fmod(tiempo * .8, 2 * pi)) + objeto,
                                                         glm::normalize(glm::vec3(1, .4f, .2f))) *
                                                   Escalar(110);
            matrices_estelas[planeta][objeto] = raiz_asteroide * Trasladar(glm::vec3(-30, 0, 0)) *
                                                glm::scale(glm::mat4(1), glm::vec3(300, 80, 80));

            // Satelite -> bisagra -> panel: todos los hijos se componen aqui con su padre.
            const int tipo = objeto % 2;
            for (int panel = 0; panel < cantidad_paneles[tipo]; ++panel)
                matrices_paneles[planeta][objeto][panel] =
                    matrices_satelites[planeta][objeto] * Trasladar(paneles[tipo][panel].pivote) *
                    Girar(glm::radians(28.0f) * sin(float(tiempo * .35) + objeto + planeta + panel * .7f),
                          paneles[tipo][panel].eje);
        }
    }
    matriz_nubes = raices_planetas[0] * Girar(Fase(tiempo, 160), glm::vec3(0, 1, 0)) *
                   Escalar(configuraciones_planetas[0].radio * 1.012f);
    // Planeta -> inclinacion orbital -> traslacion lunar -> giro propio -> escala.
    for (int luna = 0; luna < 2; ++luna)
    {
        int planeta = luna == 0 ? 0 : 2;
        matrices_lunas[luna] = raices_planetas[planeta] * Girar(glm::radians(12.0f), glm::vec3(1, 0, 0)) *
                               Girar(Fase(tiempo, 210), glm::vec3(0, 1, 0)) *
                               Trasladar(glm::vec3(configuraciones_planetas[planeta].radio * 3.3f, 0, 0)) *
                               Girar(Fase(tiempo, 160), glm::vec3(0, 1, 0)) *
                               Escalar(configuraciones_planetas[planeta].radio * .24f);
    }
}

//>>>>>>>>>>>>>>>>>>>>>> Dibujo de las jerarquias ya calculadas >>>>>>>>>>>>>>>>>>>
void EscenaEspacio::Dibujar(const ContextoRender& ctx)
{
    GLint programa = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
    auto UniformeEntero = [&](const char* n, int valor) {
        glUniform1i(glGetUniformLocation(programa, n), valor);
    };
    auto UniformeDecimal = [&](const char* n, float valor) {
        glUniform1f(glGetUniformLocation(programa, n), valor);
    };
    auto EnviarMatriz = [&](const glm::mat4& m) {
        glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(m));
    };
    UniformeDecimal("factor_noche", 1); // Luces de la estacion permanentes.
    UniformeEntero("normales_geometricas", 1);
    UniformeEntero("modo_efecto", 0);
    UniformeEntero("relieve_vibrante", 0);
    UniformeDecimal("emision_material", 0);
    UniformeDecimal("opacidad_aerea", 1);
    glUniform3f(ctx.u_color, 1, 1, 1);
    glUniform1f(ctx.u_specular_intensity, .15f);
    glUniform1f(ctx.u_shininess, 24);
    glUniform3f(glGetUniformLocation(programa, "directional_light.base.color"), 1, .96f, .9f);
    UniformeDecimal("directional_light.base.ambient_intensity", .48f);
    UniformeDecimal("directional_light.base.diffuse_intensity", .65f);
    // Estacion y naves: materiales metalicos y luminarias emisivas.
    glUniform3f(glGetUniformLocation(programa, "directional_light.direction"), -.4f, .8f, .4f);
    EnviarMatriz(matriz_estacion);
    estacion_xq6.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    EnviarMatriz(matriz_cabina);
    cabina_estacion.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    for (int i = 0; i < 15; ++i)
    {
        EnviarMatriz(matrices_naves[i]);
        naves_estacion[i].RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    }
    EnviarMatriz(matriz_aurora_exploradora);
    naves_estacion[14].RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    glUniform1f(ctx.u_specular_intensity, .15f);
    glUniform1f(ctx.u_shininess, 24);
    for (int p = 0; p < 3; ++p)
    {
        auto luz = glm::normalize(origen_estacion - configuraciones_planetas[p].centro);
        glUniform3fv(glGetUniformLocation(programa, "directional_light.direction"), 1, glm::value_ptr(luz));
        EnviarMatriz(matrices_planetas[p]);
        planetas[p].RenderModel();
        for (int j = 0; j < 5; ++j)
        {
            int tipo = j % 2;
            EnviarMatriz(matrices_satelites[p][j]);
            cuerpos_satelites[tipo].RenderModel();
            for (int k = 0; k < cantidad_paneles[tipo]; ++k)
            {
                EnviarMatriz(matrices_paneles[p][j][k]);
                paneles[tipo][k].modelo.RenderModel();
            }
            EnviarMatriz(matrices_asteroides[p][j]);
            asteroides[j % 3].RenderModel();
        }
        if (p == 0 || p == 2)
        {
            int luna = p == 0 ? 0 : 1;
            EnviarMatriz(matrices_lunas[luna]);
            lunas[luna].RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
            glUniform1f(ctx.u_specular_intensity, .15f);
            glUniform1f(ctx.u_shininess, 24);
        }
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glUniform3f(ctx.u_color, 1, 1, 1);
    UniformeDecimal("opacidad_aerea", .12f);
    EnviarMatriz(matriz_estacion);
    campos_estacion.RenderModel(true, ctx.u_specular_intensity, ctx.u_shininess);
    UniformeEntero("normales_geometricas", 1);
    UniformeDecimal("opacidad_aerea", .65f);
    textura_nubes.UseTexture();
    EnviarMatriz(matriz_nubes);
    esfera_atmosfera->RenderMeshModel();
    UniformeDecimal("opacidad_aerea", 1);
    UniformeEntero("modo_efecto", 3);
    UniformeDecimal("opacidad_particula", .27f);
    for (int p = 0; p < 3; ++p)
    {
        glUniform3fv(ctx.u_color, 1, glm::value_ptr(configuraciones_planetas[p].color_atmosfera));
        EnviarMatriz(matrices_atmosferas[p]);
        esfera_atmosfera->RenderMeshModel();
    }
    // Las matrices finales ya incluyen la jerarquia completa calculada en ActualizarMatrices().
    UniformeEntero("modo_efecto", 1);
    UniformeDecimal("opacidad_particula", .9f);
    glUniform3f(ctx.u_color, 1, 1, 1);
    textura_fuego.UseTexture();
    glUniform2f(glGetUniformLocation(programa, "desplazamiento_textura"), 0,
                -float(std::fmod(tiempo * 2, 1.0)));
    for (int p = 0; p < 3; ++p)
        for (int j = 0; j < 5; ++j)
        {
            EnviarMatriz(matrices_estelas[p][j]);
            malla_estela->RenderMeshModel();
        }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    UniformeEntero("modo_efecto", 0);
    UniformeEntero("normales_geometricas", 0);
    UniformeDecimal("opacidad_particula", 1);
    glUniform2f(glGetUniformLocation(programa, "desplazamiento_textura"), 0, 0);
    DibujarBrillos(ctx, estacion_xq6, matriz_estacion, textura_brillos);
    DibujarBrillos(ctx, cabina_estacion, matriz_cabina, textura_brillos, .8f);
    for (int i = 0; i < 15; ++i)
        DibujarBrillos(ctx, naves_estacion[i], matrices_naves[i], textura_brillos);
}

//>>>>>>>>>>>>>>>>>>>>>> Liberacion de recursos >>>>>>>>>>>>>>>>>>>
EscenaEspacio::~EscenaEspacio()
{
    for (auto& m : planetas)
        m.ClearModel();
    for (auto& m : cuerpos_satelites)
        m.ClearModel();
    for (auto& tipo : paneles)
        for (auto& p : tipo)
            p.modelo.ClearModel();
    for (auto& m : asteroides)
        m.ClearModel();
    for (auto& luna : lunas)
        luna.ClearModel();
    estacion_xq6.ClearModel();
    cabina_estacion.ClearModel();
    campos_estacion.ClearModel();
    for (auto& nave : naves_estacion)
        nave.ClearModel();
    delete esfera_atmosfera;
    delete malla_estela;
}

bool EscenaEspacio::ObjetivoCamara(int modo, glm::vec3& objetivo, glm::vec3& rumbo,
                                 float& distancia, float& altura) const
{
    if (modo != 1) return false;
    objetivo = glm::vec3(matriz_aurora_exploradora[3]);
    rumbo = glm::normalize(glm::vec3(matriz_aurora_exploradora[2]));
    distancia = 390.0f;
    altura = objetivo.y + 165.0f;
    return true;
}
