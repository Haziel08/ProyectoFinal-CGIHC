//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y cabeceras >>>>>>>>>>>>>>>>>>>>>
#include "EscenaEspacio.h"
#include <algorithm>
#include <cmath>
#include <vector>
#include <fstream>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
//>>>>>>>>>>>>>>>>>>>>>> Configuracion y funciones geometricas >>>>>>>
namespace
{
constexpr float PI = 3.14159265358979323846f;
// Solo datos inmutables que comparten la carga, la animacion y el dibujo.
const glm::vec3 origen_estacion(0, 5, 20);
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
    return float(std::fmod(tiempo / periodo, 1.0)) * 2 * PI;
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
MeshModel* CrearEsfera()
{
    std::vector<GLfloat> v;
    std::vector<unsigned> indices;
    for (int i = 0; i <= 48; ++i)
        for (int j = 0; j <= 96; ++j)
        {
            float p = PI * i / 48, a = 2 * PI * j / 96;
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
    camara = Camera(glm::vec3(0, 5, 20), glm::vec3(0, 1, 0), -90, 0, 550, .3f);
    CargarModelos();
    CargarTexturas();
    esfera_atmosfera = CrearEsfera();
    malla_estela = CrearEstela();
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
    CargarPaneles();
    luna.LoadModel("assets/espacio/modelos/luna_azul.obj", false, false);
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
                                   2 * PI * (objeto / 5.0f + planeta * .11f);
            // Circulos de latitud: radio de seccion esferica mas altura orbital.
            float radio = radio_planeta * (cos(latitud) + .38f + .03f * objeto);
            // Planeta -> giro orbital -> latitud -> orientacion -> escala del satelite.
            matrices_satelites[planeta][objeto] =
                raices_planetas[planeta] * Girar(angulo_orbital, glm::vec3(0, 1, 0)) *
                Trasladar(glm::vec3(radio, radio_planeta * sin(latitud), 0)) *
                Girar(PI / 2, glm::vec3(0, 1, 0)) * Escalar(100);
            float sector = 2 * PI * objeto / 5;
            glm::vec3 normal_region = glm::normalize(
                frente + .55f * (lateral_frente * cos(sector) + vertical_frente * sin(sector)));
            glm::vec3 lateral_region = glm::normalize(glm::cross(glm::vec3(0, 1, 0), normal_region)),
                      vertical_region = glm::cross(normal_region, lateral_region);
            float fase = Fase(tiempo, 12.0f + 2.0f * objeto) + 2 * PI * (objeto / 5.0f + planeta * .13f);
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
                                                   Girar(float(std::fmod(tiempo * .8, 2 * PI)) + objeto,
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
    // Planeta azul -> inclinacion lunar -> orbita -> distancia -> giro propio -> escala.
    matriz_luna = raices_planetas[0] * Girar(glm::radians(12.0f), glm::vec3(1, 0, 0)) *
                  Girar(Fase(tiempo, 210), glm::vec3(0, 1, 0)) *
                  Trasladar(glm::vec3(configuraciones_planetas[0].radio * 3.3f, 0, 0)) *
                  Girar(Fase(tiempo, 160), glm::vec3(0, 1, 0)) *
                  Escalar(configuraciones_planetas[0].radio * .24f);
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
        if (p == 0)
        {
            EnviarMatriz(matriz_luna);
            luna.RenderModel();
        }
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glUniform3f(ctx.u_color, 1, 1, 1);
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
    luna.ClearModel();
    delete esfera_atmosfera;
    delete malla_estela;
}
