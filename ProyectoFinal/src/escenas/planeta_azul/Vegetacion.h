#pragma once
// Vegetacion: arboles, arbustos, pasto, flores y rocas del planeta azul
// (modelos de Quaternius, Stylized Nature MegaKit, licencia CC0).
//
// - Cada modelo se carga UNA vez y se dibuja muchas veces con distinta
//   posicion, giro y tamano (instancias).
// - Las posiciones salen "al azar" pero con semilla fija: siempre queda igual.
// - Se respetan las zonas libres (para casas, NPCs...) y el lago.
// - Usa shader_vegetacion (copia de shader_light que recorta lo transparente
//   de las hojas). Al terminar deja activo el shader que estaba antes.
#include <vector>
#include <glm.hpp>
#include "Escena.h"
#include "Model.h"
#include "Material.h"
#include "Terreno.h"

// Circulo del terreno donde NO se pone vegetacion
struct ZonaLibre
{
	glm::vec2 centro;
	float radio;
	const char* nombre;
};

class Vegetacion
{
public:
	// --- Zonas despejadas (cambiar antes de Crear si se necesita otro acomodo) ---
	std::vector<ZonaLibre> zonasLibres = {
		{ glm::vec2(-110.0f, -40.0f), 50.0f, "aldea (chozas)" },
		{ glm::vec2( 110.0f, -30.0f), 45.0f, "criaturas / NPCs" },
		{ glm::vec2(   0.0f, -65.0f), 30.0f, "entrada (inicio de la camara)" },
	};
	unsigned int semilla = 7;     // cambiarla = otro acomodo al azar

	void Crear(const Terreno& terreno);
	void Dibujar(const ContextoRender& ctx, DirectionalLight* luz);
	~Vegetacion();

	bool EstaEnZonaLibre(float x, float z) const;

private:
	// Lugares donde puede ir cada grupo de plantas
	enum Lugar { TIERRA, BORDE_BOSQUE, ORILLA, FONDO_LAGO };

	// Un objeto colocado: que modelo, su matriz y su color (tinte)
	struct Instancia
	{
		int modelo;
		glm::mat4 matriz;
		glm::vec3 tinte;
	};

	// Pone "cantidad" objetos (escogidos de "modelos") separados al menos "separacion"
	void Esparcir(const std::vector<int>& modelos, int cantidad, Lugar lugar,
		float separacion, float escalaMin, float escalaMax,
		glm::vec3 tinte = glm::vec3(1.0f), float hundir = 0.0f);
	bool LugarValido(Lugar lugar, float x, float z) const;
	float Azar(float a, float b);   // numero al azar entre a y b

	const Terreno* terreno = nullptr;
	std::vector<Model*> modelos;
	std::vector<Instancia> instancias;
	Shader shader;
	Material material = Material(0.3f, 8);
	unsigned int estadoAzar = 1;
};
