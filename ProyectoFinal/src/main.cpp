/*
 * Proyecto Final — Odisea espacial y sistemas planetarios
 * Computación Gráfica e Interacción Humano-Computadora 2027-1
 *
 * main.cpp SOLO orquesta: crea la ventana, crea las escenas, elige cuál está
 * activa y la dibuja. Lo de cada zona se carga en su propia carpeta:
 *   src/escenas/espacio, planeta_naranja, planeta_morado, planeta_azul
 *
 * Teclas:
 *   1  Espacio          2  Planeta Naranja
 *   3  Planeta Morado   4  Planeta Azul
 *   W A S D + ratón      mover la cámara libre de la escena activa
 *   ESC                  salir
 */
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <vector>
#include <string>

#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "Window.h"
#include "Shader_light.h"
#include "Camera.h"

#include "Escena.h"
#include "espacio/EscenaEspacio.h"
#include "planeta_naranja/EscenaPlanetaNaranja.h"
#include "planeta_morado/EscenaPlanetaMorado.h"
#include "planeta_azul/EscenaPlanetaAzul.h"

int main()
{
	Window ventana(1366, 768);
	if (ventana.Initialise() != 0)
		return 1;

	Shader shader;
	shader.CreateFromFiles("shaders/shader_light.vert", "shaders/shader_light.frag");

	// El orden de esta lista es el de las teclas 1, 2, 3, 4 para cambiar de escena
	std::vector<Escena*> escenas = {
		new EscenaEspacio(),
		new EscenaPlanetaNaranja(),
		new EscenaPlanetaMorado(),
		new EscenaPlanetaAzul()
	};
	for (Escena* e : escenas)
		e->Inicializar();

	const int teclasEscena[] = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4 };
	int escenaActual = 0;

	auto ActualizarTitulo = [&]() {
		std::string t = std::string("Proyecto Final CGeIHC  |  ") + escenas[escenaActual]->Nombre();
		ventana.setTitulo(t.c_str());
	};
	ActualizarTitulo();

	GLfloat ultimoTiempo = (GLfloat)glfwGetTime();

	while (!ventana.getShouldClose())
	{
		// Tiempo en segundos entre frames
		GLfloat ahora = (GLfloat)glfwGetTime();
		GLfloat dt = ahora - ultimoTiempo;
		ultimoTiempo = ahora;
		if (dt > 0.1f) dt = 0.1f;

		ventana.NuevoFrame();
		glfwPollEvents();

		// Cambio de escena
		for (int i = 0; i < 4 && i < (int)escenas.size(); i++)
		{
			if (ventana.TeclaPresionada(teclasEscena[i]) && i != escenaActual)
			{
				escenaActual = i;
				ActualizarTitulo();
			}
		}

		Escena* escena = escenas[escenaActual];
		escena->Actualizar(dt, ventana);

		// Cámara libre de la escena activa
		Camera& camara = escena->ObtenerCamara();
		camara.keyControl(ventana.getsKeys(), dt);
		camara.mouseControl(ventana.getXChange(), ventana.getYChange());

		glm::mat4 vista = camara.calculateViewMatrix();
		glm::vec3 ojo = camara.getCameraPosition();
		glm::mat4 proyeccion = glm::perspective(glm::radians(60.0f),
			ventana.getBufferWidth() / ventana.getBufferHeight(), 0.1f, 2000.0f);

		// Dibujo
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		escena->DibujarSkybox(vista, proyeccion);

		shader.UseShader();
		glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(proyeccion));
		glUniformMatrix4fv(shader.GetViewLocation(), 1, GL_FALSE, glm::value_ptr(vista));
		glUniform3f(shader.GetEyePositionLocation(), ojo.x, ojo.y, ojo.z);
		escena->AplicarLuces(shader);

		ContextoRender ctx;
		ctx.uModel = shader.GetModelLocation();
		ctx.uColor = shader.getColorLocation();
		ctx.uSpecularIntensity = shader.GetSpecularIntensityLocation();
		ctx.uShininess = shader.GetShininessLocation();
		escena->Dibujar(ctx);

		glUseProgram(0);
		ventana.swapBuffers();
	}

	for (Escena* e : escenas)
		delete e;
	return 0;
}
