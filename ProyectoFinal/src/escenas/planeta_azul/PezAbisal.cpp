#include "PezAbisal.h"

namespace PlanetaAzul
{
//>>>>>>>>>>>>>>>>>>>>>> PezAbisal >>>>>>>>>>>>>>>>>>>>>>


// Articulaciones respecto al centro del cuerpo (en metros, antes de escalar)
static const glm::vec3 piv_aleta_izq(-0.38f, -0.36f, -0.16f);
static const glm::vec3 piv_aleta_der( 0.48f, -0.30f, -0.11f);
static const glm::vec3 piv_farol    ( 0.06f,  1.19f,  0.64f);

// Recorrido: elipse en lo hondo del lago, siempre un poco arriba del fondo
static const float centro_x = -10.0f, centro_z = 65.0f;
static const float radio_x = 38.0f, radio_z = 24.0f;
static const float velocidad = 0.07f;       // radianes por segundo
static const float sobre_fondo = 5.0f;      // metros arriba del fondo
static const float escala = 2.4f;

void PezAbisal::Crear(const Terreno& t)
{
	terreno = &t;
	cuerpo.LoadModel("assets/planeta_azul/modelos/pez_abisal_cuerpo.obj");
	aleta_izq.LoadModel("assets/planeta_azul/modelos/pez_abisal_aleta_izq.obj");
	aleta_der.LoadModel("assets/planeta_azul/modelos/pez_abisal_aleta_der.obj");
	farol.LoadModel("assets/planeta_azul/modelos/pez_abisal_farol.obj");

	// Halo del farol: un cuadrado que siempre mira a la camara
	halo = Texture("assets/planeta_azul/texturas/halo_farol.png");
	halo.LoadTextureA();
	GLfloat vertices[] = { -.5f,-.5f,0, 0,0, 0,0,-1,   .5f,-.5f,0, 1,0, 0,0,-1,
	                        .5f, .5f,0, 1,1, 0,0,-1,  -.5f, .5f,0, 0,1, 0,0,-1 };
	unsigned int indices[] = { 0,1,2, 0,2,3 };
	plano_halo.CreateMeshModel(vertices, indices, 32, 6);
}

void PezAbisal::Actualizar(float dt, Window& ventana)
{
	tiempo += dt;
	angulo += velocidad * dt;

	// Tecla L prender / apagar la linterna
	if (ventana.TeclaPresionada(GLFW_KEY_L))
		linterna = !linterna;

	// Posicion en la elipse, siguiendo el fondo del lago (sin salirse del agua)
	posicion.x = centro_x + radio_x * cosf(angulo);
	posicion.z = centro_z + radio_z * sinf(angulo);
	posicion.y = glm::min(terreno->Altura(posicion.x, posicion.z) + sobre_fondo, -6.0f)
		+ 0.4f * sinf(tiempo * 1.3f);

	// Rumbo = derivada de la elipse
	giro = atan2f(-radio_x * sinf(angulo), radio_z * cosf(angulo));

	// El reflector sale del farol hacia adelante y un poco hacia abajo
	glm::mat4 m = MatrizCuerpo();
	glm::vec3 origen = glm::vec3(m * glm::vec4(piv_farol, 1.0f));
	glm::vec3 direccion = glm::normalize(glm::vec3(m * glm::vec4(0.0f, -0.6f, 1.0f, 0.0f)));
	foco = SpotLight(0.65f, 0.95f, 1.0f,          // azul claro
		0.0f, 4.0f,                              // ambiental, difusa
		origen.x, origen.y, origen.z,
		direccion.x, direccion.y, direccion.z,
		1.0f, 0.022f, 0.0019f,                   // atenuacion (alcance de unos 60 m)
		28.0f);                                  // abertura del cono en grados
}

glm::mat4 PezAbisal::MatrizCuerpo() const
{
	float balanceo = glm::radians(6.0f) * sinf(tiempo * 2.0f);   // nada moviendose de lado a lado
	glm::mat4 model(1.0f);
	model = glm::translate(model, posicion);
	model = glm::rotate(model, giro + balanceo, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(escala));
	return model;
}

void PezAbisal::Dibujar(const ContextoRender& ctx)
{
	glm::vec3 blanco(1.0f);
	glUniform3fv(ctx.u_color, 1, glm::value_ptr(blanco));

	float aleteo = glm::radians(25.0f) * sinf(tiempo * 4.0f);

	// --- Cuerpo (padre) ---
	glm::mat4 model = MatrizCuerpo();
	glm::mat4 modelaux = model;
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	cuerpo.RenderModel();

	// --- Aletas: suben y bajan en sentidos contrarios ---
	model = modelaux;
	model = glm::translate(model, piv_aleta_izq);
	model = glm::rotate(model, aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_izq.RenderModel();

	model = modelaux;
	model = glm::translate(model, piv_aleta_der);
	model = glm::rotate(model, -aleteo, glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	aleta_der.RenderModel();

	// --- Farol: si la linterna esta prendida se dibuja sin sombra (modo 2 = color plano) ---
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	GLint modo = glGetUniformLocation(programa, "modo_efecto");
	model = modelaux;
	model = glm::translate(model, piv_farol);
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	if (linterna) glUniform1i(modo, 2);
	farol.RenderModel();
	glUniform1i(modo, 0);
}

void PezAbisal::DibujarHalo(const ContextoRender& ctx)
{
	if (!linterna) return;
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	GLint modo = glGetUniformLocation(programa, "modo_efecto");
	GLint opacidad = glGetUniformLocation(programa, "opacidad_particula");

	// Cuadrado en el farol girado hacia la camara (inversa de la rotacion de la vista)
	glm::vec3 p = glm::vec3(MatrizCuerpo() * glm::vec4(piv_farol, 1.0f));
	glm::mat4 model = glm::transpose(glm::mat4(glm::mat3(ctx.vista)));
	model[3] = glm::vec4(p, 1.0f);
	model = glm::scale(model, glm::vec3(2.5f + 0.2f * sinf(tiempo * 5.0f)));   // late un poco

	glUniform1i(modo, 4);
	glUniform1f(opacidad, 0.9f);
	glUniform3f(ctx.u_color, 0.6f, 0.95f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);          // se suma: brilla
	glDepthMask(GL_FALSE);
	halo.UseTexture();
	glUniformMatrix4fv(ctx.u_model, 1, GL_FALSE, glm::value_ptr(model));
	plano_halo.RenderMeshModel();
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glUniform1i(modo, 0);
	glUniform1f(opacidad, 1.0f);
	glUniform3f(ctx.u_color, 1.0f, 1.0f, 1.0f);
}
}
