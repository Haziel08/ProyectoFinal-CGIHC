#include "AvatarMarine.h"

//>>>>>>>>>>>>>>>>>>>>>> AvatarMarine >>>>>>>>>>>>>>>>>>>>>>

// La animacion de correr dura 1.07 s; mas lenta se ve como trote
static const float velocidad_animacion = 0.75f;

void AvatarMarine::Cargar()
{
	// Malla con huesos + animacion de correr (normalizado: mide 1 de alto y los pies en y = 0)
	cargado = modelo.Cargar("assets/avatar/modelos/marine.glb", "assets/avatar/texturas/marine.jpg", true);
	if (cargado)
		shader.CreateFromFiles("shaders/shader_esqueleto.vert", "shaders/shader_light.frag");
}

void AvatarMarine::Actualizar(float dt)
{
	tiempo += dt;
}

void AvatarMarine::Dibujar(const ContextoRender& ctx, const Escena& escena, DirectionalLight* luz,
	PointLight* luces, unsigned int num_luces, SpotLight* focos, unsigned int num_focos)
{
	glm::vec3 posicion, rumbo;
	float estatura;
	if (!cargado || !escena.CaminoAvatar(tiempo, posicion, rumbo, estatura))
		return;

	GLint shader_main = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader_main);

	// Mismos uniforms que main.cpp manda al shader de luz
	shader.UseShader();
	glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(ctx.proyeccion));
	glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(ctx.vista));
	glUniform3f(shader.GetEyePositionLocation(), ctx.pos_camara.x, ctx.pos_camara.y, ctx.pos_camara.z);
	shader.SetDirectionalLight(luz);
	shader.SetPointLights(luces, num_luces);
	shader.SetSpotLights(focos, num_focos);
	glUniform3f(shader.getColorLocation(), 1.0f, 1.0f, 1.0f);
	glUniform1f(shader.GetSpecularIntensityLocation(), 0.6f);
	glUniform1f(shader.GetShininessLocation(), 32.0f);

	// Trasladar -> girar hacia donde camina -> escalar a la estatura de la escena
	glm::mat4 model(1.0f);
	model = glm::translate(model, posicion);
	model = glm::rotate(model, atan2f(rumbo.x, rumbo.z), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(estatura)) * modelo.Normalizacion();
	glUniformMatrix4fv(shader.GetModelLocation(), 1, GL_FALSE, glm::value_ptr(model));

	modelo.CalcularPose(0, tiempo * velocidad_animacion);
	GLint programa = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &programa);
	modelo.Dibujar((GLuint)programa);

	glUseProgram(shader_main);
}

bool AvatarMarine::Camara(const Escena& escena, glm::vec3& objetivo, glm::vec3& rumbo, float& distancia, float& altura) const
{
	glm::vec3 posicion;
	float estatura;
	if (!cargado || !escena.CaminoAvatar(tiempo, posicion, rumbo, estatura))
		return false;
	// Todo en proporcion a la estatura para que se vea igual en cualquier escena
	objetivo = posicion + glm::vec3(0.0f, estatura * 0.7f, 0.0f);
	distancia = estatura * 2.8f;
	altura = posicion.y + estatura * 1.4f;
	return true;
}
