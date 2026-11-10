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
#include "PezAbisal.h"
#include "ArbolesLuminosos.h"

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

	// main.cpp usa las luces puntuales mas cercanas (fogata, antorchas y arboles)
	PointLight* ObtenerLucesPuntuales() override { return luces; }
	unsigned int NumLucesPuntuales() override { return num_luces; }

	// Reflector del pez abisal
	SpotLight* ObtenerFocos() override { return &pez_abisal.foco; }
	unsigned int NumFocos() override { return pez_abisal.linterna ? 1 : 0; }

	bool CaminoAvatar(float tiempo, glm::vec3& posicion, glm::vec3& rumbo, float& estatura) const override;

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
	PezAbisal pez_abisal;
	ArbolesLuminosos arboles;
	Agua agua;
    Model lampara_suelo;
    Texture halo_suelo;
    MeshModel malla_halo_suelo, malla_halo_tejado;
    void CargarLamparas();
    void DibujarLamparas(const ContextoRender& ctx);
    float nivel_agua = -1.0f;      // altura de la superficie del lago

	// El shader solo acepta 16 luces puntuales: de todas se mandan las 16 mas cercanas a la camara
	PointLight luces[max_point_lights];
	unsigned int num_luces = 0;
	glm::vec3 ultima_camara{ 0.0f };
	void EscogerLuces();

};
}

using EscenaPlanetaAzul = PlanetaAzul::EscenaPlanetaAzul;
