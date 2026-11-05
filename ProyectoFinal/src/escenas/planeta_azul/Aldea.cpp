#include "Aldea.h"

#include <cmath>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// Posiciones RELATIVAS al centro de la aldea: x, z y giro en grados
struct PuntoAldea
{
	float x, z, giro;
};

// Chozas en circulo (radio 26), mirando hacia la fogata
static const PuntoAldea posChozas[4] = {
	{  18.4f,  18.4f, 225.0f },
	{ -18.4f,  18.4f, 135.0f },
	{ -18.4f, -18.4f,  45.0f },
	{  18.4f, -18.4f, 315.0f },
};

// Antorchas: 3 alrededor de la fogata y 2 en la entrada de la aldea (todas con luz)
static const int numAntorchas = 5;
static const PuntoAldea posAntorchas[numAntorchas] = {
	{  13.0f,   0.0f, 0.0f },
	{  -6.5f,  11.3f, 0.0f },
	{  -6.5f, -11.3f, 0.0f },
	{  40.0f,  -8.0f, 0.0f },
	{  40.0f,   8.0f, 0.0f },
};

static const float altoAntorcha = 3.4f;   // donde empieza la llama (punta de la antorcha)

glm::vec3 Aldea::SobreElSuelo(float x, float z) const
{
	return glm::vec3(x, terreno->Altura(x, z), z);
}

void Aldea::Crear(const Terreno& t)
{
	terreno = &t;

	ChozaMuu_M.LoadModel("assets/planeta_azul/modelos/Choza_Muu.obj");
	ChozaMantah_M.LoadModel("assets/planeta_azul/modelos/Choza_Mantah.obj");
	ChozaKrakee_M.LoadModel("assets/planeta_azul/modelos/Choza_Krakee.obj");
	ChozaAntuk_M.LoadModel("assets/planeta_azul/modelos/Choza_Antuk.obj");
	Antorcha_M.LoadModel("assets/planeta_azul/modelos/Antorcha.obj");
	Fogata_M.LoadModel("assets/planeta_azul/modelos/Fogata.obj");

	fuego.Crear();
}

void Aldea::Actualizar(float tiempo, bool esDeNoche, Window& ventana)
{
	// Tecla F: prender / apagar la fogata
	if (ventana.TeclaPresionada(GLFW_KEY_F))
		fogataEncendida = !fogataEncendida;

	// Se vuelve a llenar el arreglo de luces cada frame con las que esten prendidas
	numLuces = 0;

	if (fogataEncendida)
	{
		glm::vec3 p = SobreElSuelo(centro.x, centro.y) + glm::vec3(0.0f, 1.0f, 0.0f);
		float parpadeo = 0.3f * sinf(tiempo * 9.0f);           // la luz tiembla como fuego
		luces[numLuces++] = PointLight(1.0f, 0.55f, 0.2f,        // color naranja
			0.0f, 3.0f + parpadeo,                               // ambiental, difusa
			p.x, p.y, p.z,                                       // posicion
			1.0f, 0.045f, 0.0075f);                              // atenuacion (alcance ~30 m)
	}

	antorchasEncendidas = esDeNoche;
	if (antorchasEncendidas)
	{
		for (int i = 0; i < numAntorchas; i++)
		{
			glm::vec3 p = SobreElSuelo(centro.x + posAntorchas[i].x, centro.y + posAntorchas[i].z)
				+ glm::vec3(0.0f, altoAntorcha, 0.0f);
			float parpadeo = 0.2f * sinf(tiempo * 12.0f + i);
			luces[numLuces++] = PointLight(1.0f, 0.6f, 0.25f,
				0.0f, 1.5f + parpadeo,
				p.x, p.y, p.z,
				1.0f, 0.09f, 0.032f);                            // alcance ~15 m
		}
	}
}

void Aldea::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// --- Chozas ---
	Model* chozas[4] = { &ChozaMuu_M, &ChozaMantah_M, &ChozaKrakee_M, &ChozaAntuk_M };
	for (int i = 0; i < 4; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, SobreElSuelo(centro.x + posChozas[i].x, centro.y + posChozas[i].z)
			- glm::vec3(0.0f, 0.3f, 0.0f));                     // un poco enterrada por si hay desnivel
		model = glm::rotate(model, glm::radians(posChozas[i].giro), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		chozas[i]->RenderModel();
	}

	// --- Antorchas ---
	for (int i = 0; i < numAntorchas; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, SobreElSuelo(centro.x + posAntorchas[i].x, centro.y + posAntorchas[i].z));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		Antorcha_M.RenderModel();
	}

	// --- Fogata ---
	glm::mat4 model(1.0f);
	model = glm::translate(model, SobreElSuelo(centro.x, centro.y));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	Fogata_M.RenderModel();
}

void Aldea::DibujarLlamas(const ContextoRender& ctx, float tiempo)
{
	// Llama chica en la punta de cada antorcha (solo de noche)
	if (antorchasEncendidas)
	{
		for (int i = 0; i < numAntorchas; i++)
		{
			glm::vec3 p = SobreElSuelo(centro.x + posAntorchas[i].x, centro.y + posAntorchas[i].z);
			fuego.Dibujar(ctx, p + glm::vec3(0.0f, altoAntorcha - 0.1f, 0.0f), 0.6f, 1.0f, tiempo);
		}
	}

	// Llama grande de la fogata (solo si esta prendida)
	if (fogataEncendida)
		fuego.Dibujar(ctx, SobreElSuelo(centro.x, centro.y) + glm::vec3(0.0f, 0.2f, 0.0f), 2.0f, 3.0f, tiempo);
}
