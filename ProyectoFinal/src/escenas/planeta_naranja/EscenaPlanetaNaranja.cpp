#include "EscenaPlanetaNaranja.h"

void EscenaPlanetaNaranja::Inicializar()
{
	CargarSkybox("assets/planeta_naranja/skybox/");

	// Cámara libre de esta escena: posición inicial, vector arriba, yaw, pitch,
	// velocidad (unidades/segundo) y sensibilidad del ratón
	camara = Camera(glm::vec3(0.0f, 5.0f, 20.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 20.0f, 0.3f);

	// Aquí se cargarán modelos, texturas y luces de la escena
}

void EscenaPlanetaNaranja::Actualizar(float dt, Window& ventana)
{
	// Aquí van las animaciones y las teclas propias de la escena
}

void EscenaPlanetaNaranja::Dibujar(const ContextoRender& ctx)
{
	// Aquí se dibujan los objetos de la escena
}
