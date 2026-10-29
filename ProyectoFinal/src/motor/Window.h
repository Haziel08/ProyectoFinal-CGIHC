#pragma once
// Ventana GLFW, estados de teclado y desplazamiento del raton.
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

    //>>>>>>>>>>>>>>>>>>>>>> Estados de la escena >>>>>>>>>>>>>>>>>>>>>>>
    int getEscenaActiva() const { return escenaActiva; }
    bool getLuzNaveEncendida() const { return luzNaveEncendida; }
    bool getMotor1Encendido() const { return motor1Encendido; }
    bool getMotor2Encendido() const { return motor2Encendido; }
    bool getVistaGeneralSolicitada() const { return vistaGeneralSolicitada; }

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
    //>>>>>>>>>>>>>>>>>>>>>> Variables de entrada >>>>>>>>>>>>>>>>>>>>>>>
    int escenaActiva;
    bool luzNaveEncendida;
    bool motor1Encendido;
    bool motor2Encendido;
    bool vistaGeneralSolicitada;

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
