#pragma once
#include "Escena.h"
#include "Model.h"
#include "Material.h"
#include <glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <cmath>
#include <string>
#include <vector>
#include "CicloDiaNoche.h"
#include "Terreno.h"
#include "Agua.h"
#include "Vegetacion.h"
#include "Aldea.h"
#include "Peces.h"
#include "Necromorfos.h"
#include "Stalker.h"
#include "Pidgeys.h"
#include "Reaper.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> EscenaPlanetaAzul >>>>>>>>>>>>>>>>>>>>>>

class EscenaPlanetaAzul : public Escena
{
public:
	const char* Nombre() const override { return "Planeta Azul"; }
    ~EscenaPlanetaAzul() override { lampara_suelo.ClearModel(); }
    bool ObjetivoCamara(int modo, glm::vec3& objetivo, glm::vec3& rumbo, float& distancia, float& altura) const override;
    void Inicializar() override;
	void Actualizar(float dt, Window& ventana) override;
	void Dibujar(const ContextoRender& ctx) override;

	// main.cpp usa esta luz cuando el planeta azul esta activo
	DirectionalLight* ObtenerLuzDireccional() override { return &luz_sol; }

	// main.cpp usa las luces de la aldea fogata y antorchas
	PointLight* ObtenerLucesPuntuales() override { return aldea.luces; }
	unsigned int NumLucesPuntuales() override { return aldea.num_luces; }

private:
	float tiempo = 0.0f;          // reloj de la escena

	CicloDiaNoche ciclo;
	DirectionalLight luz_sol;      // la unica luz direccional del planeta
	Skybox skybox_dia;
	Skybox skybox_noche;

	Terreno terreno;
	Vegetacion vegetacion;
	Aldea aldea;
	Peces peces;
	Necromorfos necromorfos;
	Stalker stalker;             // avatar
	Pidgeys pidgeys;
	Reaper reaper;
	Agua agua;
    Model lampara_suelo;
    Texture halo_suelo;
    MeshModel malla_halo_suelo, malla_halo_tejado;
    void CargarLamparas();
    void DibujarLamparas(const ContextoRender& ctx);
    float nivel_agua = -1.0f;      // altura de la superficie del lago

};
}

using EscenaPlanetaAzul = PlanetaAzul::EscenaPlanetaAzul;
