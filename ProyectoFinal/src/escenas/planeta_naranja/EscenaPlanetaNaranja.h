#pragma once

//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y tipos del motor >>>>>>>>>>>>>>>
#include <array>
#include <vector>
#include "Escena.h"
#include "Mesh_tn.h"
#include "Model.h"
#include "Texture.h"

//>>>>>>>>>>>>>>>>>>>>>> Interfaz de la escena >>>>>>>>>>>>>>>>>>>>>>>
class EscenaPlanetaNaranja : public Escena
{
public:
    ~EscenaPlanetaNaranja() override;
    const char* Nombre() const override
    {
        return "Planeta Naranja";
    }
    void Inicializar() override;
    void Actualizar(float dt, Window& ventana) override;
    void Dibujar(const ContextoRender& ctx) override;
    glm::mat4 ObtenerMatrizNave() const;
    glm::mat4 ObtenerMatrizNaveEspiral() const;
    glm::vec3 ObtenerPosicionLuzPoste(int indice, const glm::vec3& ojo) const;

private:
    //>>>>>>>>>>>>>>>>>>>>>> Constantes compartidas de la escena >>>>>>>>>
    static constexpr float escala_pista = 0.18f;
    static constexpr float escala_nave = 1.1f;
    static constexpr float altura_suelo = -3900.0f;
    static constexpr float diametro_asteroide = 64.0f;
    static constexpr float escala_dron = 9.0f;
    static constexpr float vida_humo_aereo = 2.0f;
    static constexpr float intervalo_humo_aereo = 0.40f;
    static constexpr int max_humo_aereo = 3 * (int(vida_humo_aereo / intervalo_humo_aereo) + 1);

    //>>>>>>>>>>>>>>>>>>>>>> Tipos internos >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    using TrianguloPista = std::array<glm::vec3, 3>;
    struct MontanaPeriferica
    {
        glm::mat4 matriz;
        int modelo;
    };
    struct ConfiguracionDron
    {
        glm::vec3 centro;
        float largo, ancho, giro, sentido, fase;
    };
    struct ParticulaHumoAereo
    {
        glm::vec3 posicion{0.0f};
        float edad = vida_humo_aereo;
        float tamano = 24.0f;
        int asteroide = 0;
    };

    //>>>>>>>>>>>>>>>>>>>>>> Modelos y mallas >>>>>>>>>>>>>>>>>>>>>>>>>>>>
    Model suelo, pista, pista_espiral, nave;
    Model poste_luz, soporte_faro_lateral, soportes_y_senalizacion;
    Model torre_comunicaciones, torre_control;
    Model montanas[3], roca_periferica;
    Model asteroides[3], dron_explorador;
    MeshModel* malla_fuego = nullptr;
    MeshModel* malla_estela_asteroide = nullptr;
    MeshModel* malla_humo_aereo = nullptr;
    MeshModel* malla_aspas_dron = nullptr;

    //>>>>>>>>>>>>>>>>>>>>>> Texturas de los efectos >>>>>>>>>>>>>>>>>>>>>
    Texture* fuego = nullptr;
    Texture* textura_fuego_asteroide = nullptr;
    Texture* textura_humo = nullptr;
    Texture* textura_aspas = nullptr;

    //>>>>>>>>>>>>>>>>>>>>>> Distribucion y matrices persistentes >>>>>>>>
    glm::mat4 matriz_pista{1.0f}, matriz_nave{1.0f}, matriz_nave_espiral{1.0f};
    std::vector<glm::mat4> matrices_postes, apoyos_faroles_elevados, rocas_perifericas;
    std::vector<MontanaPeriferica> montanas_perifericas;
    std::vector<TrianguloPista> secciones_pista, secciones_espiral;
    std::array<glm::mat4, 3> matrices_asteroides;
    std::array<glm::mat4, 5> matrices_drones;
    std::array<glm::mat4, 3> matrices_cuerpos_asteroides, matrices_estelas_asteroides;
    glm::mat4 matrices_rotores[5][2];
    glm::mat4 matrices_fuegos[2][2];

    //>>>>>>>>>>>>>>>>>>>>>> Estado de la animacion >>>>>>>>>>>>>>>>>>>>>>
    std::vector<float> distancias;
    float longitud_circuito = 0.0f;
    float avance_nave = 0.0f, tiempo_espiral = 0.0f;
    float tiempo_aereo = 0.0f, angulo_aspas = 0.0f, emision_humo_aereo = 0.0f;
    bool motor1 = true, motor2 = true;
    std::array<ParticulaHumoAereo, max_humo_aereo> humo_aereo;
    int siguiente_humo_aereo = 0;

    //>>>>>>>>>>>>>>>>>>>>>> Carga y distribucion inicial >>>>>>>>>>>>>>>>
    void CargarModelos();
    void CargarTexturas();
    void CrearFuego();
    void CrearMallasAereas();
    void CrearRecorrido();
    void CrearFaros();
    void CrearPaisajePeriferico();

    //>>>>>>>>>>>>>>>>>>>>>> Geometria y curvas auxiliares >>>>>>>>>>>>>>>
    static glm::mat4 MarcoDeVuelo(const glm::vec3& posicion, const glm::vec3& tangente);
    static glm::mat4 MatrizAsteroide(float tiempo, int indice);
    static float VisibilidadAsteroide(const glm::mat4& matriz);
    static ConfiguracionDron ConfigurarDron(int indice);
    static glm::mat4 MatrizDron(float tiempo, int indice);
    static std::vector<TrianguloPista> LeerSeccionesPista(const char* ruta);
    bool FaroInvadeTableroSuperior(const glm::vec3& base, const glm::vec3& hacia_dentro) const;
    glm::vec2 BordesEnSeccion(const glm::vec3& centro, const glm::vec3& adelante,
                              const glm::vec3& lateral) const;
    static float DistanciaCircuitoPlanta(const glm::vec3& punto);
    static glm::vec3 PuntoTablero(float tiempo, float carril);
    glm::vec3 PuntoPista(float distancia) const;

    //>>>>>>>>>>>>>>>>>>>>>> Animacion y dibujo jerarquico >>>>>>>>>>>>>>>
    void ActualizarNaves(float dt);
    void ActualizarObjetosAereos(float dt);
    void DibujarNaves(const ContextoRender& ctx);
    void DibujarObjetosAereos(const ContextoRender& ctx);
};
