#pragma once
/*
 * Window.h
 * Ventana GLFW + entrada de teclado y ratón.
 *
 * IMPORTANTE (para no chocar entre integrantes):
 *   Aquí YA NO se agregan variables por cada animación/objeto como en las
 *   prácticas (muevex, articulacion1..18, etc.). Cada escena lee el teclado
 *   por su cuenta dentro de su método Actualizar() usando:
 *
 *     ventana.TeclaSostenida(GLFW_KEY_W)   -> true mientras la tecla esté abajo
 *     ventana.TeclaPresionada(GLFW_KEY_E)  -> true SOLO en el frame en que se
 *                                             presionó (ideal para encender /
 *                                             apagar luces o iniciar animaciones)
 */
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise(const char* titulo = "Proyecto Final CGeIHC");

	// Buffers y estado de la ventana
	GLfloat getBufferWidth() { return (GLfloat)bufferWidth; }
	GLfloat getBufferHeight() { return (GLfloat)bufferHeight; }
	bool getShouldClose() { return glfwWindowShouldClose(mainWindow); }
	void swapBuffers() { glfwSwapBuffers(mainWindow); }
	void setTitulo(const char* titulo) { glfwSetWindowTitle(mainWindow, titulo); }

	// Teclado
	bool* getsKeys() { return keys; }               // usado por Camera::keyControl
	bool TeclaSostenida(int key) const;
	bool TeclaPresionada(int key) const;
	void NuevoFrame();                               // llamar ANTES de glfwPollEvents()

	// Ratón (cuánto se movió desde la última lectura)
	GLfloat getXChange();
	GLfloat getYChange();

	~Window();

private:
	GLFWwindow* mainWindow;
	GLint width, height;
	GLint bufferWidth, bufferHeight;

	bool keys[1024];          // tecla abajo
	bool keysPressed[1024];   // tecla presionada en este frame

	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	bool mouseFirstMoved;

	void createCallbacks();
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);
	static void ManejaResize(GLFWwindow* window, int w, int h);
};
