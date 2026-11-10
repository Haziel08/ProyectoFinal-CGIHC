#include "Aldea.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Aldea >>>>>>>>>>>>>>>>>>>>>>


// Posiciones al centro de la aldea
struct PuntoAldea
{
	float x, z, giro;
};

// Chozas mirando hacia la fogata 4 cerca y 3 mas lejos.
// La entrada de la aldea queda libre del lado +X.
static const int num_chozas = 7;
static const PuntoAldea pos_chozas[num_chozas] = {
	{  18.4f,  18.4f, 225.0f },
	{ -18.4f,  18.4f, 135.0f },
	{ -18.4f, -18.4f,  45.0f },
	{  18.4f, -18.4f, 315.0f },
	{   0.0f,  40.0f, 180.0f },
	{ -40.0f,   0.0f,  90.0f },
	{   0.0f, -40.0f,   0.0f },
};

// Antorchas
//   3 junto a la fogata, 2 en la entrada y 8 en un circulo de radio 33
static const int num_antorchas = 13;
static const PuntoAldea pos_antorchas[num_antorchas] = {
	{  13.0f,   0.0f, 0.0f }, {  -6.5f,  11.3f, 0.0f }, {  -6.5f, -11.3f, 0.0f },
	{  40.0f,  -8.0f, 0.0f }, {  40.0f,   8.0f, 0.0f },
	{  30.5f,  12.6f, 0.0f }, {  12.6f,  30.5f, 0.0f }, { -12.6f,  30.5f, 0.0f }, { -30.5f,  12.6f, 0.0f },
	{ -30.5f, -12.6f, 0.0f }, { -12.6f, -30.5f, 0.0f }, {  12.6f, -30.5f, 0.0f }, {  30.5f, -12.6f, 0.0f },
};

static const float alto_antorcha = 3.4f;   // donde empieza la llama

int Aldea::NumChozas() const
{
	return num_chozas;
}

glm::vec2 Aldea::PosicionChoza(int i) const
{
	return centro + glm::vec2(pos_chozas[i].x, pos_chozas[i].z);
}

glm::vec3 Aldea::SobreElSuelo(float x, float z) const
{
	return glm::vec3(x, terreno->Altura(x, z), z);
}

void Aldea::Crear(const Terreno& t)
{
	terreno = &t;

	choza_muu.LoadModel("assets/planeta_azul/modelos/choza_muu.obj");
	choza_mantah.LoadModel("assets/planeta_azul/modelos/choza_mantah.obj");
	choza_krakee.LoadModel("assets/planeta_azul/modelos/choza_krakee.obj");
	choza_antuk.LoadModel("assets/planeta_azul/modelos/choza_antuk.obj");
	antorcha.LoadModel("assets/planeta_azul/modelos/antorcha.obj");
	fogata.LoadModel("assets/planeta_azul/modelos/fogata.obj");

	fuego.Crear();
}

void Aldea::Actualizar(float tiempo, bool es_de_noche, Window& ventana)
{
	// Tecla Y prender / apagar la fogata
	if (ventana.TeclaPresionada(GLFW_KEY_Y))
		fogata_encendida = !fogata_encendida;

	// Se vuelve a llenar el arreglo de luces cada frame con las que esten prendidas
	num_luces = 0;

	if (fogata_encendida)
	{
		glm::vec3 p = SobreElSuelo(centro.x, centro.y) + glm::vec3(0.0f, 1.0f, 0.0f);
		float parpadeo = 0.3f * sinf(tiempo * 9.0f);           // la luz tiembla como fuego
		pos_luces[num_luces] = p;
		luces[num_luces++] = PointLight(1.0f, 0.55f, 0.2f,        // color naranja
			0.0f, 3.0f + parpadeo,                               // ambiental, difusa
			p.x, p.y, p.z,                                       // posicion
			1.0f, 0.045f, 0.007f);                              // atenuacion
	}

	// Al anochecer se prenden solas y al amanecer se apagan;
	// en cualquier momento la tecla K las prende o apaga a mano
	if (es_de_noche != era_de_noche)
	{
		antorchas_encendidas = es_de_noche;
		era_de_noche = es_de_noche;
	}
	if (ventana.TeclaPresionada(GLFW_KEY_K))
		antorchas_encendidas = !antorchas_encendidas;

	if (antorchas_encendidas)
	{
		for (int i = 0; i < num_antorchas && num_luces < max_point_lights; i++)
		{
			glm::vec3 p = SobreElSuelo(centro.x + pos_antorchas[i].x, centro.y + pos_antorchas[i].z)
				+ glm::vec3(0.0f, alto_antorcha, 0.0f);
			float parpadeo = 0.2f * sinf(tiempo * 12.0f + i);
			pos_luces[num_luces] = p;
			luces[num_luces++] = PointLight(1.0f, 0.6f, 0.25f,
				0.0f, 1.5f + parpadeo,
				p.x, p.y, p.z,
				1.0f, 0.09f, 0.032f);                            // alcance
		}
	}
}

void Aldea::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	//Chozas
	Model* chozas[4] = { &choza_muu, &choza_mantah, &choza_krakee, &choza_antuk };
	for (int i = 0; i < num_chozas; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, SobreElSuelo(centro.x + pos_chozas[i].x, centro.y + pos_chozas[i].z)
			- glm::vec3(0.0f, 0.3f, 0.0f));                     // un poco enterrada por si hay desnivel
		model = glm::rotate(model, glm::radians(pos_chozas[i].giro), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		chozas[i % 4]->RenderModel();      // se repiten los 4 modelos de choza
	}

	//Antorchas
	for (int i = 0; i < num_antorchas; i++)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, SobreElSuelo(centro.x + pos_antorchas[i].x, centro.y + pos_antorchas[i].z));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		antorcha.RenderModel();
	}

	//Fogata
	glm::mat4 model(1.0f);
	model = glm::translate(model, SobreElSuelo(centro.x, centro.y));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	fogata.RenderModel();
}

void Aldea::DibujarLlamas(const ContextoRender& ctx, float tiempo)
{
	// Llama en la punta de cada antorcha
	if (antorchas_encendidas)
	{
		for (int i = 0; i < num_antorchas; i++)
		{
			glm::vec3 p = SobreElSuelo(centro.x + pos_antorchas[i].x, centro.y + pos_antorchas[i].z);
			fuego.Dibujar(ctx, p + glm::vec3(0.0f, alto_antorcha - 0.1f, 0.0f), 0.6f, 1.0f, tiempo);
		}
	}

	// Llama grande de la fogata solo si esta prendida
	if (fogata_encendida)
		fuego.Dibujar(ctx, SobreElSuelo(centro.x, centro.y) + glm::vec3(0.0f, 0.2f, 0.0f), 2.0f, 3.0f, tiempo);
}
}
