#include "Vegetacion.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> Vegetacion >>>>>>>>>>>>>>>>>>>>>>


// Posiciones
struct Lugar
{
	float x, z, escala, giro;
};

//rboles sueltos en el pasto
static const Lugar arboles[] = {
	{ -40, -110, 2.0f,   0 }, {  45, -115, 1.8f,  60 }, { -150,  60, 2.2f, 120 },
	{ -120, 110, 1.9f, 200 }, { 130,   80, 2.1f,  30 }, {  120, 130, 1.8f, 270 },
	{ -60,  150, 2.0f,  90 }, {  60,  150, 2.2f, 150 }, {    0, 150, 1.9f, 300 },
	{ -30,  -20, 1.7f,  45 }, {  40,  -10, 1.8f, 210 }, { -135, 140, 2.0f, 330 },
};

//Arbustos
static const Lugar arbustos[] = {
	{ -25, -35, 1.6f,   0 }, {  30, -40, 1.4f,  80 }, { -55, -100, 1.8f, 160 },
	{  60, -90, 1.5f, 240 }, { -100, 40, 1.7f,  20 }, {  100,  40, 1.6f, 100 },
	{ -90, 120, 1.8f, 180 }, {  95, 105, 1.5f, 260 }, {  -20, 140, 1.6f, 340 },
	{  25, 140, 1.4f,  50 }, { -145, 15, 1.7f, 130 }, {  145,  10, 1.6f, 210 },
};

//Rocas en el pasto
static const Lugar rocas[] = {
	{ -70, -130, 3.0f,   0 }, {  80, -135, 2.5f,  70 }, { -140,  90, 3.5f, 140 },
	{ 140,   60, 2.8f, 210 }, { -10, -120, 2.0f, 280 }, {   50, -60, 1.8f, 350 },
	{ -75,   20, 2.2f,  40 }, { 150, -110, 3.2f, 110 }, { -150, -130, 3.0f, 180 },
	{  10,  160, 2.6f, 250 },
};

//Rocas en la orilla del lago
static const Lugar rocas_orilla[] = {
	{  90, 93, 1.8f,  0 }, { -74, 112, 2.2f, 60 }, { -90, 47, 1.6f, 120 },
	{ -33,  8, 2.0f, 180 }, {  74,  28, 1.7f, 240 }, {   0, 136, 2.4f, 300 },
};

//Dentro del lago
static const Lugar rocas_lago[] = {
	{ -30, 60, 4.0f, 0 }, { 25, 80, 3.0f, 70 }, { 0, 100, 4.5f, 140 },
	{  45, 55, 3.5f, 210 }, { -50, 85, 3.0f, 280 }, { 10, 45, 2.5f, 350 },
};
static const Lugar algas[] = {
	{ -15, 70, 1.4f,  0 }, { 20, 62, 1.2f,  45 }, { 35, 90, 1.5f,  90 }, { -40, 75, 1.3f, 135 },
	{  -5, 95, 1.6f, 180 }, { 55, 75, 1.2f, 225 }, { -60, 60, 1.4f, 270 }, { 15, 105, 1.5f, 315 },
};

bool Vegetacion::EnZonaLibre(float x, float z) const
{
	// Aldea
	if (glm::distance(glm::vec2(x, z), glm::vec2(-110.0f, -40.0f)) < 50.0f) return true;
	if (glm::distance(glm::vec2(x, z), glm::vec2(110.0f, -30.0f)) < 45.0f) return true;
	if (glm::distance(glm::vec2(x, z), glm::vec2(0.0f, -65.0f)) < 30.0f) return true;
	return false;
}

void Vegetacion::Crear(const Terreno& t)
{
	terreno = &t;

	arbol1.LoadModel("assets/planeta_azul/modelos/arbol_hojas_1.obj");
	arbol2.LoadModel("assets/planeta_azul/modelos/arbol_hojas_2.obj");
	pino1.LoadModel("assets/planeta_azul/modelos/pino_1.obj");
	pino2.LoadModel("assets/planeta_azul/modelos/pino_2.obj");
	arbusto.LoadModel("assets/planeta_azul/modelos/arbusto.obj");
	arbusto_flores.LoadModel("assets/planeta_azul/modelos/arbusto_flores.obj");
	pasto.LoadModel("assets/planeta_azul/modelos/pasto_alto.obj");
	flores.LoadModel("assets/planeta_azul/modelos/flores.obj");
	roca1.LoadModel("assets/planeta_azul/modelos/roca_mediana_1.obj");
	roca2.LoadModel("assets/planeta_azul/modelos/roca_mediana_2.obj");
	piedra.LoadModel("assets/planeta_azul/modelos/piedra_redonda.obj");
	alga.LoadModel("assets/planeta_azul/modelos/alga.obj");

	shader.CreateFromFiles("shaders/shader_light.vert", "shaders/shader_vegetacion.frag");
}

void Vegetacion::DibujarEn(Model& modelo, float x, float z, float escala, float giro, glm::vec3 color)
{
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(x, terreno->Altura(x, z), z));   // sobre el suelo
	model = glm::rotate(model, glm::radians(giro), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escala));
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(u_color, 1, glm::value_ptr(color));
	modelo.RenderModel();
}

void Vegetacion::Dibujar(const ContextoRender& ctx, DirectionalLight* luz, PointLight* luces, unsigned int num_luces)
{
	// Guardar el shader de main para regresar a el al final
	GLint shader_main = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader_main);

	// Mismos uniforms que main.cpp le manda a shader_light
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(ctx.proyeccion));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(ctx.vista));
	glUniform3f(shader.GetEyePositionLocation(), ctx.pos_camara.x, ctx.pos_camara.y, ctx.pos_camara.z);
	shader.SetDirectionalLight(luz);
	shader.SetPointLights(luces, num_luces);      // fogata y antorchas
	shader.SetSpotLights(nullptr, 0);
	material_plantas.UseMaterial(shader.GetSpecularIntensityLocation(), shader.GetShininessLocation());
	u_model = shader.GetModelLocation();
	u_color = shader.getColorLocation();

	//Bosque alrededor del terreno dos filas de arboles en cada lad
	Model* tipos_arbol[4] = { &arbol1, &arbol2, &pino1, &pino2 };
	int i = 0;
	for (float t = -180.0f; t <= 180.0f; t += 10.0f)
	{
		float escala = 1.8f + (i % 3) * 0.4f;   // tamanos variados
		float giro = i * 47.0f;
		// fila de afuera y fila de adentro
		DibujarEn(*tipos_arbol[i % 4],       t, -178.0f, escala, giro);
		DibujarEn(*tipos_arbol[(i + 1) % 4], t,  178.0f, escala, giro);
		DibujarEn(*tipos_arbol[(i + 2) % 4], -178.0f, t, escala, giro);
		DibujarEn(*tipos_arbol[(i + 3) % 4],  178.0f, t, escala, giro);
		float u = t + 5.0f;
		if (!EnZonaLibre(u, -163.0f)) DibujarEn(*tipos_arbol[(i + 2) % 4], u, -163.0f, escala, giro + 90);
		if (!EnZonaLibre(u,  163.0f)) DibujarEn(*tipos_arbol[(i + 3) % 4], u,  163.0f, escala, giro + 90);
		if (!EnZonaLibre(-163.0f, u)) DibujarEn(*tipos_arbol[i % 4], -163.0f, u, escala, giro + 90);
		if (!EnZonaLibre( 163.0f, u)) DibujarEn(*tipos_arbol[(i + 1) % 4],  163.0f, u, escala, giro + 90);
		i++;
	}

	// Arboles, arbustos y rocas
	int k = 0;
	for (const Lugar& l : arboles)       // se alternan: arbol, pino, arbol, pino...
		DibujarEn(k++ % 2 == 0 ? arbol1 : pino1, l.x, l.z, l.escala, l.giro);
	k = 0;
	for (const Lugar& l : arbustos)      // se alternan: arbusto, arbusto con flores...
		DibujarEn(k++ % 2 == 0 ? arbusto : arbusto_flores, l.x, l.z, l.escala, l.giro);
	for (const Lugar& l : rocas)    DibujarEn(roca1, l.x, l.z, l.escala, l.giro, glm::vec3(1.3f));
	for (const Lugar& l : rocas_orilla) DibujarEn(roca2, l.x, l.z, l.escala, l.giro, glm::vec3(1.3f));

	//Pasto y flores
	int n = 0;
	for (float x = -165.0f; x <= 165.0f; x += 15.0f)
	{
		for (float z = -165.0f; z <= 165.0f; z += 15.0f)
		{
			n++;
			float px = x + (n % 3) * 4.0f;      // se recorre un poco para que no se vea en fila
			float pz = z + (n % 4) * 3.0f;
			if (terreno->DistanciaLago(px, pz) < 1.2f || EnZonaLibre(px, pz))
				continue;
			if (n % 3 == 0)
				DibujarEn(flores, px, pz, 1.3f, n * 30.0f);
			else
				DibujarEn(pasto, px, pz, 1.6f, n * 30.0f);
			DibujarEn(piedra, px + 6.0f, pz + 2.0f, 3.0f, n * 50.0f);   // una piedrita junto
		}
	}

	//Fondo del lago: rocas y algas
	for (const Lugar& l : rocas_lago) DibujarEn(roca1, l.x, l.z, l.escala, l.giro, glm::vec3(0.7f, 0.85f, 0.95f));
	for (const Lugar& l : algas)     DibujarEn(alga, l.x, l.z, l.escala, l.giro, glm::vec3(0.45f, 0.85f, 0.7f));

	glUseProgram(shader_main);
}
}
