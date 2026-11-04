#include "EscenaPlanetaAzul.h"

void EscenaPlanetaAzul::Inicializar()
{
	// --- Skybox de dia y de noche ---
	// Se cargan sin voltear (otras escenas pueden dejar activado el volteo de stb)
	stbi_set_flip_vertically_on_load(false);
	CargarSkybox("assets/planeta_azul/skybox/noche/");
	skyboxNoche = skybox;
	CargarSkybox("assets/planeta_azul/skybox/dia/");
	skyboxDia = skybox;

	// --- Camara: empieza en la orilla mirando hacia +Z (hacia el sol y el lago) ---
	camara = Camera(glm::vec3(0.0f, 10.0f, -60.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, -8.0f, 20.0f, 0.3f);

	// --- Suelo y lago ---
	terreno.Crear();
	glm::vec2 tamanoLago = terreno.radioLago * 2.2f;   // un poco mas grande que el lago
	agua.Crear(terreno.centroLago, tamanoLago, nivelAgua, "assets/planeta_azul/texturas/agua.jpg");

	vegetacion.Crear(terreno);
	aldea.Crear(terreno);

	luzSol = ciclo.LuzDireccional();

	// Aquí se cargarán los modelos de la escena
}

void EscenaPlanetaAzul::Actualizar(float dt, Window& ventana)
{
	tiempo += dt;

	// Ciclo de dia y noche: actualiza la luz y escoge el skybox
	ciclo.Actualizar(dt);
	luzSol = ciclo.LuzDireccional();
	skybox = ciclo.EsDeNoche() ? skyboxNoche : skyboxDia;

	// Aldea: tecla F para la fogata y antorchas que se prenden de noche
	aldea.Actualizar(tiempo, ciclo.EsDeNoche(), ventana);

	// Aquí van las animaciones y las teclas propias de la escena
}

void EscenaPlanetaAzul::Dibujar(const ContextoRender& ctx)
{
	// 1) Todo lo opaco primero
	terreno.Dibujar(ctx);
	aldea.Dibujar(ctx);

	// Aquí se dibujan los modelos (chozas, criaturas, cosas bajo el agua...)

	// 2) Plantas y rocas (usa su propio shader y luego regresa al de main)
	vegetacion.Dibujar(ctx, &luzSol, aldea.luces, aldea.numLuces);

	// 3) El agua siempre al final (es transparente)
	agua.Dibujar(ctx, tiempo, ciclo.Luz());

	// 4) El fuego al final de todo (brilla encima de lo que tiene atras)
	aldea.DibujarLlamas(ctx, tiempo);
}
