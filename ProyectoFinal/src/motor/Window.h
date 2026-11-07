#pragma once
// Ventana GLFW, estados de teclado y desplazamiento del raton.
#include <stdio.h>
#include <glew.h>
#include <glfw3.h>

class Window
{
public:
	Window();
	Window(GLint window_width, GLint window_height);
	int Initialise(const char* titulo = "Proyecto Final CGeIHC");

	// Buffers y estado de la ventana
	GLfloat getBufferWidth() { return (GLfloat)buffer_width; }
	GLfloat getBufferHeight() { return (GLfloat)buffer_height; }
	bool getShouldClose() { return glfwWindowShouldClose(main_window); }
	void swapBuffers() { glfwSwapBuffers(main_window); }
	void setTitulo(const char* titulo) { glfwSetWindowTitle(main_window, titulo); }

    //>>>>>>>>>>>>>>>>>>>>>> Estados de la escena >>>>>>>>>>>>>>>>>>>>>>>
    int getEscenaActiva() const { return escena_activa; }
    bool getLuzNaveEncendida() const { return luz_nave_encendida; }
    bool getMotor1Encendido() const { return motor1_encendido; }
    bool getMotor2Encendido() const { return motor2_encendido; }
    bool getVistaGeneralSolicitada() const { return vista_general_solicitada; }

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
    int escena_activa;
    bool luz_nave_encendida;
    bool motor1_encendido;
    bool motor2_encendido;
    bool vista_general_solicitada;

	GLFWwindow* main_window;
	GLint width, height;
	GLint buffer_width, buffer_height;

	bool keys[1024];          // tecla abajo
	bool keys_pressed[1024];   // tecla presionada en este frame

	GLfloat last_x;
	GLfloat last_y;
	GLfloat x_change;
	GLfloat y_change;
	bool mouse_first_moved;

	void createCallbacks();
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double x_pos, double y_pos);
	static void ManejaResize(GLFWwindow* window, int w, int h);
};
