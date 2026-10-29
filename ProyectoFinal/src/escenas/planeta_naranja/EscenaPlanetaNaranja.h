#pragma once
#include "Escena.h"

//>>>>>>>>>>>>>>>>>>>>>> Interfaz del planeta naranja >>>>>>>>>>>>>>>
class EscenaPlanetaNaranja : public Escena
{
public:
    ~EscenaPlanetaNaranja() override;
    const char* Nombre() const override { return "Planeta Naranja"; }
    void Inicializar() override;
    void Actualizar(float dt, Window& ventana) override;
    void Dibujar(const ContextoRender& ctx) override;
    glm::mat4 ObtenerMatrizNave() const;

private:
    void CrearSuelo();
    void CrearFuego();
    glm::vec3 PuntoPista(float distancia) const;
    void ActualizarNave(float dt);
    void DibujarFuego(const ContextoRender& ctx, float lado);
};
