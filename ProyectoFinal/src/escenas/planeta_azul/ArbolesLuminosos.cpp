#include "ArbolesLuminosos.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> ArbolesLuminosos >>>>>>>>>>>>>>>>>>>>>>


struct PuntoArbol
{
	float x, z, escala, giro;
};

// En la parte honda del lago; el pez abisal pasa entre ellos
static const PuntoArbol pos_arboles[ArbolesLuminosos::num_arboles] = {
	{   6.0f,  71.0f, 10.0f,   0.0f }, { -26.0f,  74.0f,  9.0f,  60.0f },
	{  -9.0f,  53.0f, 11.0f, 130.0f }, {  37.0f,  70.0f,  9.5f, 200.0f },
	{  12.0f,  94.0f,  8.5f, 250.0f }, { -36.0f,  97.0f, 10.0f, 310.0f },
	{ -48.0f,  52.0f,  9.0f,  20.0f }, {  24.0f,  44.0f,  8.0f, 170.0f },
};

static const float alto_copa = 0.75f;      // donde va la luz (fraccion de la altura del arbol)

void ArbolesLuminosos::Crear(const Terreno& t)
{
	terreno = &t;
	// Mismo modelo que el planeta morado (no se duplica en el proyecto)
	arbol.LoadModel("assets/planeta_morado/modelos/arbol_luminoso.obj", false, false);
}

void ArbolesLuminosos::Actualizar(float dt, bool es_de_noche, Window& ventana)
{
	tiempo += dt;

	// Igual que las antorchas: el ciclo los prende al anochecer y la tecla J a mano
	if (es_de_noche != era_de_noche)
	{
		encendidos = es_de_noche;
		era_de_noche = es_de_noche;
	}
	if (ventana.TeclaPresionada(GLFW_KEY_J))
		encendidos = !encendidos;

	// El brillo sube o baja poco a poco (no de golpe)
	float objetivo = encendidos ? 1.0f : 0.0f;
	brillo += (objetivo - brillo) * glm::min(1.0f, 2.0f * dt);

	// Una luz morada en la copa de cada arbol
	num_luces = 0;
	if (brillo < 0.02f) return;
	for (int i = 0; i < num_arboles; i++)
	{
		const PuntoArbol& a = pos_arboles[i];
		glm::vec3 p(a.x, terreno->Altura(a.x, a.z) + a.escala * alto_copa, a.z);
		float pulso = 0.25f * sinf(tiempo * 1.5f + i);          // laten despacio
		pos_luces[num_luces] = p;
		luces[num_luces++] = PointLight(0.7f, 0.3f, 1.0f,     // morado
			0.0f, (2.0f + pulso) * brillo,
			p.x, p.y, p.z,
			1.0f, 0.07f, 0.017f);                             // alcance de unos 40 m
	}
}

void ArbolesLuminosos::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	// Los frutos azules brillan con "brillo_frutos" del shader_light
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	GLint frutos = glGetUniformLocation(programa, "brillo_frutos");
	glUniform1f(frutos, 0.3f + 3.4f * brillo);

	for (int i = 0; i < num_arboles; i++)
	{
		const PuntoArbol& a = pos_arboles[i];
		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(a.x, terreno->Altura(a.x, a.z) - 0.2f, a.z));
		model = glm::rotate(model, glm::radians(a.giro), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(a.escala));
		glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
		arbol.RenderModel();
	}
	glUniform1f(frutos, 0.0f);
}
}
