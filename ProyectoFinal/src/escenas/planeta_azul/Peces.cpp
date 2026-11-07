#include "Peces.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Peces >>>>>>>>>>>>>>>>>>>>>>


// Recorrido de cada Air fish (forma de 8)
struct RecorridoOcho
{
	float cx, cz;          // centro del 8
	float ancho_x, largo_z;  // tamano del 8
	float profundidad;     // altura
	float velocidad;       // que tan rapido recorre el 8
	float fase;            // para que no empiecen todos en el mismo punto
	float escala;
};

static const RecorridoOcho peces_aire[] = {
	//  cx    cz   ancho_x largo_z  prof   vel    fase  escala
	{   0.0f, 70.0f, 40.0f, 22.0f,  -6.0f, 0.12f, 0.0f, 1.5f },
	{ -10.0f, 75.0f, 30.0f, 18.0f, -12.0f, 0.15f, 2.0f, 1.2f },
	{  15.0f, 65.0f, 35.0f, 20.0f, -18.0f, 0.10f, 4.0f, 1.8f },
};

// Cardumen de Peepers todos dan vueltas al mismo circulo, uno detras de otro
static const float centro_cardumen_x = 10.0f, centro_cardumen_z = 80.0f;
static const float radio_cardumen = 18.0f;
static const float velocidad_cardumen = 0.3f;
static const int num_peepers = 6;

void Peces::Crear()
{
	air_fish.LoadModel("assets/planeta_azul/modelos/pez_aire.obj");
	peeper.LoadModel("assets/planeta_azul/modelos/peeper.obj");
}

void Peces::Actualizar(float dt)
{
	tiempo += dt;
}

void Peces::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	//Air fish forma de 8
	for (const RecorridoOcho& r : peces_aire)
	{
		float t = tiempo * r.velocidad + r.fase;

		// Posicion sobre el 8
		float x = r.cx + r.ancho_x * sinf(t);
		float z = r.cz + r.largo_z * sinf(2.0f * t) / 2.0f;
		float y = r.profundidad + 0.8f * sinf(3.0f * t);          // sube y baja

		// Direccion de avance = derivada de la posicion
		float dx = r.ancho_x * cosf(t);
		float dz = r.largo_z * cosf(2.0f * t);
		float giro = atan2f(dx, dz);                              // mirar hacia donde avanza
		float coleteo = glm::radians(10.0f) * sinf(tiempo * 6.0f + r.fase);

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(x, y, z));
		model = glm::rotate(model, giro + coleteo, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(r.escala));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		air_fish.RenderModel();
	}

	//Peepers cardumen en circulo
	for (int i = 0; i < num_peepers; i++)
	{
		float t = tiempo * velocidad_cardumen - i * 0.35f;   // cada uno va un poco atras del anterior
		float radio = radio_cardumen + (i % 3) * 2.0f;        // no todos en la misma linea
		float x = centro_cardumen_x + radio * cosf(t);
		float z = centro_cardumen_z + radio * sinf(t);
		float y = -10.0f - (i % 2) * 2.5f + 0.5f * sinf(tiempo * 2.0f + i);

		// Direccion de avance en el circulo: (-sen t, cos t)
		float giro = atan2f(-sinf(t), cosf(t));
		float coleteo = glm::radians(12.0f) * sinf(tiempo * 9.0f + i);

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(x, y, z));
		model = glm::rotate(model, giro + coleteo, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		peeper.RenderModel();
	}
}
}
