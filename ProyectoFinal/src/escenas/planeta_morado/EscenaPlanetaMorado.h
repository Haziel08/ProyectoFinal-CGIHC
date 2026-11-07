#pragma once
#include "Escena.h"
#include "Model.h"
#include "Texture.h"
#include <vector>
#include <array>
#include "planeta_azul/EscenaPlanetaAzul.h"

class CoheteAuxiliar
{
public:
    void Cargar();
    void Actualizar(float dt, bool iniciar);
    void Dibujar(const ContextoRender& ctx, glm::vec3 base, float noche);
    ~CoheteAuxiliar();
private:
    Model cabina, cuerpo, motor, plataforma;
    std::vector<glm::vec4> cuadros;
    float tiempo = 0;
    bool activo = false;
};

class EscenaPlanetaMorado : public Escena
{
public:
    const char* Nombre() const override { return "Planeta Morado"; }
    void Inicializar() override;
    void Actualizar(float dt, Window& ventana) override;
    void Dibujar(const ContextoRender& ctx) override;
    DirectionalLight* ObtenerLuzDireccional() override { return &luz; }
    ~EscenaPlanetaMorado() override;

private:
    struct Instancia
    {
        int modelo;
        glm::mat4 matriz;
    };
    std::vector<Instancia> poblado, vegetacion, sierra;
    Model refugios[2], flora[5], montanas[3], meteoritos[3];
    Model sonda_cuerpo, sonda_rotor, cohete, motores[3], plataforma, neptuno[3];
    MeshModel suelo, base_inferior, base_media, base_lanzamiento, caminos, plano_estela, malla_estela;
    Texture textura_suelo, textura_camino, textura_estela, textura_humo;
    Skybox cielo_dia, cielo_noche;
    DirectionalLight luz;
    glm::mat4 matriz_sonda{1}, matriz_cohete{1}, matrices_motores[3];
    float tiempo = 0.0f, tiempo_cohete = 0.0f;
    bool lanzamiento = false;
    float factor_noche = 0;
    CoheteAuxiliar auxiliar;
    struct CuadroCohete
    {
        float duracion;
        int motor;
        struct Pieza
        {
            int acoplada;
            glm::vec3 posicion, angulos;
        };
        std::array<Pieza,5> piezas;
    };
    std::vector<CuadroCohete> cuadros_cohete;
    std::array<Model,5> segundo_cohete;
    std::array<glm::mat4,5> matrices_segundo;
    size_t cuadro_segundo = 0, lazo_segundo = 0;
    float tiempo_segundo = 0.0f;
    bool segundo_activo = false;
    PlanetaAzul::ModeloAnimado avatar;
    Shader shader_avatar;
    bool avatar_cargado = false;
    void CargarSegundoCohete(const std::string& ruta);
    void ActualizarSegundoCohete(float dt);
    void DibujarAvatar(const ContextoRender& ctx);
};
