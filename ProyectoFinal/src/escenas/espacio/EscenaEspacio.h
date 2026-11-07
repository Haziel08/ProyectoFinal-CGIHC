#pragma once
//>>>>>>>>>>>>>>>>>>>>>> Bibliotecas y tipos del motor >>>>>>>>>>>>>>>
#include "Escena.h"

#include "Mesh_tn.h"
#include "Texture.h"
#include "Model.h"

class EscenaEspacio : public Escena
{
public:
    //>>>>>>>>>>>>>>>>>>>>>> Interfaz de la escena >>>>>>>>>>>>>>>>>>>>>>>
    ~EscenaEspacio() override;
    const char* Nombre() const override
    {
        return "Espacio";
    }
    bool ObjetivoCamara(int modo, glm::vec3& objetivo, glm::vec3& rumbo, float& distancia, float& altura) const override;
    void Inicializar() override;
    void Actualizar(float dt, Window& ventana) override;
    void Dibujar(const ContextoRender& ctx) override;
    float PlanoCercano() const override
    {
        return 2.0f;
    }
    float AlcanceVista() const override
    {
        return 40000.0f;
    }

private:
    //>>>>>>>>>>>>>>>>>>>>>> Tipo del panel articulado >>>>>>>>>>>>>>>>>>>
    struct PanelOrbital
    {
        Model modelo;
        glm::vec3 pivote{0.0f};
        glm::vec3 eje{1.0f, 0.0f, 0.0f};
    };
    //>>>>>>>>>>>>>>>>>>>>>> Modelos y paneles >>>>>>>>>>>>>>>>>>>>>>>>>>>>
    Model planetas[3];
    Model cuerpos_satelites[2];
    PanelOrbital paneles[2][8];
    int cantidad_paneles[2] = {0, 0};
    Model asteroides[3];
    Model lunas[2];
    Model estacion_xq6, cabina_estacion, campos_estacion;
    Model naves_estacion[15];
    //>>>>>>>>>>>>>>>>>>>>>> Texturas y mallas de efectos >>>>>>>>>>>>>>>>>
    Texture textura_nubes;
    Texture textura_fuego;
    Texture textura_brillos;
    MeshModel* esfera_atmosfera = nullptr;
    MeshModel* malla_estela = nullptr;
    //>>>>>>>>>>>>>>>>>>>>>> Matrices de las jerarquias >>>>>>>>>>>>>>>>>>>
    glm::mat4 raices_planetas[3];
    glm::mat4 matrices_planetas[3];
    glm::mat4 matrices_satelites[3][5];
    glm::mat4 matrices_asteroides[3][5];
    glm::mat4 matrices_estelas[3][5];
    glm::mat4 matrices_paneles[3][5][8];
    glm::mat4 matrices_atmosferas[3];
    glm::mat4 matriz_nubes{1.0f};
    glm::mat4 matrices_lunas[2];
    glm::mat4 matriz_estacion{1.0f};
    glm::mat4 matriz_cabina{1.0f};
    glm::mat4 matrices_naves[15];
    //>>>>>>>>>>>>>>>>>>>>>> Estado de la animacion >>>>>>>>>>>>>>>>>>>>>>>
    double tiempo = 0.0;

    //>>>>>>>>>>>>>>>>>>>>>> Carga y actualizacion de jerarquias >>>>>>>>>>>
    void CargarModelos();
    void CargarPaneles();
    void CargarTexturas();
    glm::mat4 matriz_aurora_exploradora{1.0f};
    void ActualizarMatrices();
};
