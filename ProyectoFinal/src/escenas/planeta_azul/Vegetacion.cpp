#include "Vegetacion.h"

#include <cmath>
#include <string>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Indices de los modelos (mismo orden que la lista "archivos" de abajo)
enum
{
	ARBOL_1, ARBOL_2, ARBOL_3, PINO_1, PINO_2, PINO_3,
	ARBUSTO, ARBUSTO_FLORES, PASTO_CORTO, PASTO_ALTO, FLORES, FLOR,
	HELECHO, PLANTA_GRANDE, HONGO, ROCA_1, ROCA_2, ROCA_3,
	PIEDRA_1, PIEDRA_2, PIEDRA_3, TOTAL_MODELOS
};

static const char* archivos[TOTAL_MODELOS] = {
	"CommonTree_3", "CommonTree_4", "CommonTree_5", "Pine_2", "Pine_4", "Pine_5",
	"Bush_Common", "Bush_Common_Flowers", "Grass_Common_Short", "Grass_Common_Tall",
	"Flower_3_Group", "Flower_4_Single", "Fern_1", "Plant_1_Big", "Mushroom_Common",
	"Rock_Medium_1", "Rock_Medium_2", "Rock_Medium_3",
	"Pebble_Round_1", "Pebble_Round_3", "Pebble_Square_2"
};

Vegetacion::~Vegetacion()
{
	for (Model* m : modelos)
		delete m;
}

float Vegetacion::Azar(float a, float b)
{
	// Generador sencillo (siempre da la misma secuencia con la misma semilla)
	estadoAzar = estadoAzar * 1664525u + 1013904223u;
	float t = (estadoAzar >> 8) / 16777216.0f;      // 0..1
	return a + (b - a) * t;
}

bool Vegetacion::EstaEnZonaLibre(float x, float z) const
{
	for (const ZonaLibre& zona : zonasLibres)
		if (glm::distance(glm::vec2(x, z), zona.centro) < zona.radio)
			return true;
	return false;
}

bool Vegetacion::LugarValido(Lugar lugar, float x, float z) const
{
	float d = terreno->DistanciaLago(x, z);     // <1 lago, 1..1.12 playa, >1.12 pasto
	bool enTierra = d > 1.2f && !EstaEnZonaLibre(x, z);
	switch (lugar)
	{
	case TIERRA:       return enTierra;
	case BORDE_BOSQUE: return enTierra && glm::max(fabsf(x), fabsf(z)) > 135.0f;
	case ORILLA:       return d > 1.1f && d < 1.35f && !EstaEnZonaLibre(x, z);
	case FONDO_LAGO:   return d < 0.85f;
	}
	return false;
}

void Vegetacion::Esparcir(const std::vector<int>& lista, int cantidad, Lugar lugar,
	float separacion, float escalaMin, float escalaMax, glm::vec3 tinte, float hundir)
{
	std::vector<glm::vec2> puestos;
	float limite = terreno->tamano / 2.0f - 5.0f;

	// Se prueban lugares al azar; se acepta el que cumpla las reglas
	for (int intento = 0; intento < cantidad * 30 && (int)puestos.size() < cantidad; intento++)
	{
		float x = Azar(-limite, limite);
		float z = Azar(-limite, limite);
		if (!LugarValido(lugar, x, z))
			continue;

		bool muyCerca = false;
		for (const glm::vec2& p : puestos)
			if (glm::distance(p, glm::vec2(x, z)) < separacion) { muyCerca = true; break; }
		if (muyCerca)
			continue;
		puestos.push_back(glm::vec2(x, z));

		// Matriz: llevar al suelo -> girar en Y -> escalar
		float escala = Azar(escalaMin, escalaMax);
		float y = terreno->Altura(x, z) - hundir * escala;
		glm::mat4 m(1.0f);
		m = glm::translate(m, glm::vec3(x, y, z));
		m = glm::rotate(m, glm::radians(Azar(0.0f, 360.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
		m = glm::scale(m, glm::vec3(escala));

		int modelo = lista[(int)Azar(0.0f, (float)lista.size() - 0.001f)];
		instancias.push_back({ modelo, m, tinte });
	}
}

void Vegetacion::Crear(const Terreno& t)
{
	terreno = &t;
	estadoAzar = semilla;

	// Cargar cada modelo una sola vez
	for (int i = 0; i < TOTAL_MODELOS; i++)
	{
		Model* m = new Model();
		m->LoadModel(std::string("assets/planeta_azul/modelos/") + archivos[i] + ".obj");
		modelos.push_back(m);
	}

	shader.CreateFromFiles("shaders/shader_light.vert", "shaders/shader_vegetacion.frag");

	std::vector<int> arboles = { ARBOL_1, ARBOL_2, ARBOL_3, PINO_1, PINO_2, PINO_3 };
	std::vector<int> pastos = { PASTO_CORTO, PASTO_ALTO };
	std::vector<int> flores = { FLORES, FLOR };
	std::vector<int> rocas = { ROCA_1, ROCA_2, ROCA_3 };
	std::vector<int> piedras = { PIEDRA_1, PIEDRA_2, PIEDRA_3 };
	glm::vec3 tinteAgua(0.70f, 0.85f, 0.95f);           // un poco azul bajo el agua
	glm::vec3 tinteAlga(0.45f, 0.85f, 0.70f);
	glm::vec3 tinteRoca(1.3f);                          // aclara la textura de roca

	//        modelos            cant  lugar          separ  escala   [tinte, hundir]
	// Bosque alrededor del terreno y algunos arboles sueltos
	Esparcir(arboles,            480, BORDE_BOSQUE,  6.0f, 1.6f, 2.8f);
	Esparcir(arboles,             45, TIERRA,       16.0f, 1.2f, 2.0f);
	// Plantas bajas
	Esparcir({ ARBUSTO, ARBUSTO_FLORES }, 80, TIERRA, 5.0f, 1.2f, 2.0f);
	Esparcir(pastos,             500, TIERRA,        1.5f, 1.2f, 2.0f);
	Esparcir(flores,             140, TIERRA,        2.5f, 1.0f, 1.6f);
	Esparcir({ HELECHO },         40, TIERRA,        6.0f, 0.4f, 0.6f);
	Esparcir({ PLANTA_GRANDE },   30, TIERRA,        6.0f, 0.8f, 1.3f);
	Esparcir({ HONGO },           25, TIERRA,        3.0f, 0.8f, 1.3f);
	// Rocas en tierra y en la orilla
	Esparcir(rocas,               40, TIERRA,       10.0f, 1.5f, 4.0f, tinteRoca, 0.15f);
	Esparcir(piedras,            120, TIERRA,        2.0f, 2.0f, 4.0f);
	Esparcir(rocas,               18, ORILLA,        8.0f, 1.0f, 2.5f, tinteRoca, 0.15f);
	Esparcir(piedras,             60, ORILLA,        1.5f, 2.5f, 4.0f);
	// Fondo del lago (se ven a traves del agua)
	Esparcir(rocas,               25, FONDO_LAGO,   10.0f, 2.0f, 5.0f, tinteAgua, 0.2f);
	Esparcir({ PLANTA_GRANDE, HELECHO }, 40, FONDO_LAGO, 5.0f, 0.6f, 1.6f, tinteAlga);
	Esparcir(piedras,             60, FONDO_LAGO,    2.0f, 3.0f, 6.0f, tinteAgua);
}

void Vegetacion::Dibujar(const ContextoRender& ctx, DirectionalLight* luz)
{
	// Recordar el shader que estaba activo para regresarlo al final
	GLint shaderAnterior = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shaderAnterior);

	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(ctx.proyeccion));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(ctx.vista));
	glUniform3f(shader.GetEyePositionLocation(), ctx.posCamara.x, ctx.posCamara.y, ctx.posCamara.z);
	shader.SetDirectionalLight(luz);
	shader.SetPointLights(nullptr, 0);   // por ahora solo la luz del sol
	shader.SetSpotLights(nullptr, 0);
	material.UseMaterial(shader.GetSpecularIntensityLocation(), shader.GetShininessLocation());

	GLuint uModel = shader.GetModelLocation();
	GLuint uColor = shader.getColorLocation();
	for (const Instancia& inst : instancias)
	{
		glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(inst.matriz));
		glUniform3fv(uColor, 1, glm::value_ptr(inst.tinte));
		modelos[inst.modelo]->RenderModel();
	}

	glUseProgram(shaderAnterior);
}
