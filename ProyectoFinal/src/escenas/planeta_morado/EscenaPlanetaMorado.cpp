#include "EscenaPlanetaMorado.h"

void EscenaPlanetaMorado::Inicializar()
{
	// Skybox de dia y de noche (se cargan sin voltear)
	stbi_set_flip_vertically_on_load(false);
	CargarSkybox("assets/planeta_morado/texturas/skybox/noche/");
	skyboxNoche = skybox;
	CargarSkybox("assets/planeta_morado/texturas/skybox/dia/");
	skyboxDia = skybox;

	// Luz calida de dia y morada de noche
	ciclo.colorDia = glm::vec3(1.0f, 0.86f, 0.76f);
	ciclo.colorNoche = glm::vec3(0.55f, 0.42f, 0.85f);
	luzSol = ciclo.LuzDireccional();

	// Cámara libre: mirando hacia +Z, donde esta el sol
	camara = Camera(glm::vec3(0.0f, 5.0f, -30.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f, 20.0f, 0.3f);

	piso.Crear("assets/planeta_morado/texturas/suelo_morado.jpg");

	// Aquí se cargarán modelos, texturas y luces de la escena
}

void EscenaPlanetaMorado::Actualizar(float dt, Window& ventana)
{
	tiempo += dt;

	// Ciclo de dia y noche: cambia la luz y el skybox
	ciclo.Actualizar(dt);
	luzSol = ciclo.LuzDireccional();
	skybox = ciclo.EsDeNoche() ? skyboxNoche : skyboxDia;

	// Aquí van las animaciones y las teclas propias de la escena
}

void EscenaPlanetaMorado::Dibujar(const ContextoRender& ctx)
{
	piso.Dibujar(ctx);

	// Aquí se dibujan los objetos de la escena
}
