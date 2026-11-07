#include "Necromorfos.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Necromorfos >>>>>>>>>>>>>>>>>>>>>>


static const float escala_mixamo = 0.01f;

// La caminata
// Si el circulo los mueve a otra velocidad, los pies patinan
static const float velocidad_caminata = 1.17f;

// Que necromorfos (por numero de choza) bailan cuando se prende la fogata
static const bool bailan[] = { true, false, true, false, false, false, false };

void Necromorfos::Crear(const Terreno& t, const Aldea& aldea)
{
	terreno = &t;
	fogata = aldea.centro;

	// Malla + esqueleto + caminata (animacion 0) y despues el baile (animacion 1)
	necromorfo.Cargar("assets/planeta_azul/modelos/necromorfo_caminar.fbx", "assets/planeta_azul/texturas/necromorfo.jpg");
	anim_caminar = 0;
	anim_bailar = necromorfo.AgregarAnimacion("assets/planeta_azul/modelos/necromorfo_bailar.fbx");
	// La caminata no venia "In Place" asi que hay que moverle
	necromorfo.QuitarAvance(anim_caminar, "mixamorig:Hips");

	shader.CreateFromFiles("shaders/shader_esqueleto.vert", "shaders/shader_light.frag");

	// Uno por choza
	num_caminantes = glm::min(aldea.NumChozas(), max_caminantes);
	for (int i = 0; i < num_caminantes; i++)
	{
		glm::vec2 choza = aldea.PosicionChoza(i);
		glm::vec2 hacia_fogata = glm::normalize(fogata - choza);

		Caminante& c = caminantes[i];
		c.centro = choza + hacia_fogata * 9.0f;
		c.radio = 3.5f;
		c.angulo = i * 1.3f;                                       // cada uno empieza en otro punto
		float sentido = (i % 2 == 0) ? 1.0f : -1.0f;               // unos van al reves
		c.velocidad = sentido * velocidad_caminata / c.radio;       // v = w * r  ->  w = v / r
		c.baila_con_fogata = (i < 7) ? bailan[i] : false;
	}
}

void Necromorfos::Actualizar(float dt, bool fogata_encendida)
{
	tiempo += dt;
	bailando = fogata_encendida;

	for (int i = 0; i < num_caminantes; i++)
	{
		// Los de la aldea se quedan quietos mientras bailan
		if (bailando && caminantes[i].baila_con_fogata)
			continue;
		caminantes[i].angulo += caminantes[i].velocidad * dt;
	}
}

// Circuito peatonal cerrado con una vuelta al lago y otra a la aldea.
static glm::vec2 PuntoPatrulla(float distancia)
{
    const glm::vec2 hitos[] = {
        {-96, 71}, {-73, 128}, {0, 130}, {65, 118}, {101, 94}, {96, 68},
        {84, 17}, {44, 4}, {0, 4}, {-35, -4}, {-58, 13}, {-85, 1},
        {-130, 6}, {-154, -24}, {-150, -65}, {-111, -87}, {-72, -64},
        {-65, -32}, {-78, 1}, {-108, 27}, {-112, 53}};
    constexpr int cantidad = sizeof(hitos) / sizeof(hitos[0]);
    float longitud = 0.0f;
    for (int i = 0; i < cantidad; ++i)
        longitud += glm::distance(hitos[i], hitos[(i + 1) % cantidad]);
    distancia = std::fmod(distancia, longitud);
    if (distancia < 0) distancia += longitud;
    for (int i = 0; i < cantidad; ++i)
    {
        const glm::vec2 a = hitos[i], b = hitos[(i + 1) % cantidad];
        const float tramo = glm::distance(a, b);
        if (distancia <= tramo)
        {
            const float t = distancia / tramo;
            const glm::vec2 m = .5f * (b - hitos[(i + cantidad - 1) % cantidad]);
            const glm::vec2 n = .5f * (hitos[(i + 2) % cantidad] - a);
            return glm::mix(a, b, t * t * (3.0f - 2.0f * t)) +
                   t * (1.0f - t) * ((1.0f - t) * m - t * n);
        }
        distancia -= tramo;
    }
    return hitos[0];
}

glm::vec3 Necromorfos::PosicionPatrulla() const
{
    const glm::vec2 punto = PuntoPatrulla(tiempo * velocidad_caminata);
    return glm::vec3(punto.x, terreno->Altura(punto.x, punto.y), punto.y);
}

glm::vec3 Necromorfos::RumboPatrulla() const
{
    const glm::vec2 delta = PuntoPatrulla(tiempo * velocidad_caminata + .1f) -
                            PuntoPatrulla(tiempo * velocidad_caminata - .1f);
    return glm::normalize(glm::vec3(delta.x, 0, delta.y));
}

void Necromorfos::DibujarUno(const Caminante& c, int animacion, float tiempo_animacion, GLuint u_model)
{
	// Posicion en el circulo
	float x = c.centro.x + c.radio * cosf(c.angulo);
	float z = c.centro.y + c.radio * sinf(c.angulo);

	float giro;
	if (animacion == anim_bailar)
	{
		giro = atan2f(fogata.x - x, fogata.y - z);              // voltea hacia la fogata
	}
	else
	{
		// Direccion de avance = derivada del circulo (con el signo de la velocidad)
		float s = (c.velocidad >= 0.0f) ? 1.0f : -1.0f;
		giro = atan2f(-sinf(c.angulo) * s, cosf(c.angulo) * s);
	}

	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(x, terreno->Altura(x, z), z));
	model = glm::rotate(model, giro, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escala_mixamo));
    if (&c == caminantes)
    {
        const glm::vec3 rumbo = RumboPatrulla();
        model = glm::translate(glm::mat4(1), PosicionPatrulla());
        model = glm::rotate(model, atan2f(rumbo.x, rumbo.z), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(escala_mixamo));
        animacion = anim_caminar;
    }
    glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));

    necromorfo.CalcularPose(animacion, tiempo_animacion);
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	necromorfo.Dibujar((GLuint)programa);
}

void Necromorfos::Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int num_luces)
{
	GLint shader_main = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader_main);

	// Mismos uniforms que main.cpp manda al shader de luz
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(ctx.proyeccion));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(ctx.vista));
	glUniform3f(shader.GetEyePositionLocation(), ctx.pos_camara.x, ctx.pos_camara.y, ctx.pos_camara.z);
	shader.SetDirectionalLight(luz);
	shader.SetPointLights(luces, num_luces);
	shader.SetSpotLights(nullptr, 0);
	glUniform3f(shader.getColorLocation(), 1.0f, 1.0f, 1.0f);
	glUniform1f(shader.GetSpecularIntensityLocation(), 0.3f);
	glUniform1f(shader.GetShininessLocation(), 16.0f);

	for (int i = 0; i < num_caminantes; i++)
	{
		const Caminante& c = caminantes[i];
		bool baila = bailando && c.baila_con_fogata;
		// Cada uno con un desfase en su animacion para que no se muevan identicos
		DibujarUno(c, baila ? anim_bailar : anim_caminar, tiempo + i * 0.4f, shader.GetModelLocation());
	}

	glUseProgram(shader_main);
}
}
