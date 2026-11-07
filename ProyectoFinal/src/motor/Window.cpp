#include "Window.h"

//>>>>>>>>>>>>>>>>>>>>>> Constructores >>>>>>>>>>>>>>>>>>>>>>
Window::Window() : Window(1280, 720) {}

Window::Window(GLint window_width, GLint window_height)
{
    escena_activa = 0;
    luz_nave_encendida = true;
    motor1_encendido = true;
    motor2_encendido = true;
    vista_general_solicitada = false;
	main_window = nullptr;
	width = window_width;
	height = window_height;
	buffer_width = window_width;
	buffer_height = window_height;

	last_x = last_y = 0.0f;
	x_change = 0.0f;
	y_change = 0.0f;
	mouse_first_moved = true;

	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = false;
		keys_pressed[i] = false;
	}
}

//>>>>>>>>>>>>>>>>>>>>>> Inicializacion de GLFW y OpenGL >>>>>>>>>>>>>>>>>>>>>>
int Window::Initialise(const char* titulo)
{
	if (!glfwInit())
	{
		printf("Fallo al inicializar GLFW\n");
		glfwTerminate();
		return 1;
	}

	// OpenGL 4.3 core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	main_window = glfwCreateWindow(width, height, titulo, NULL, NULL);
	if (!main_window)
	{
		printf("Fallo al crear la ventana con GLFW\n");
		glfwTerminate();
		return 1;
	}

	glfwGetFramebufferSize(main_window, &buffer_width, &buffer_height);
	glfwMakeContextCurrent(main_window);
	createCallbacks();

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		printf("Fallo inicializacion de GLEW\n");
		glfwDestroyWindow(main_window);
		main_window = nullptr;
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, buffer_width, buffer_height);
	glfwSetWindowUserPointer(main_window, this);
	return 0;
}

//>>>>>>>>>>>>>>>>>>>>>> Registro de eventos >>>>>>>>>>>>>>>>>>>>>>
void Window::createCallbacks()
{
	glfwSetKeyCallback(main_window, ManejaTeclado);
	glfwSetCursorPosCallback(main_window, ManejaMouse);
	glfwSetFramebufferSizeCallback(main_window, ManejaResize);
}

bool Window::TeclaSostenida(int key) const
{
	return key >= 0 && key < 1024 && keys[key];
}

bool Window::TeclaPresionada(int key) const
{
	return key >= 0 && key < 1024 && keys_pressed[key];
}

void Window::NuevoFrame()
{
    vista_general_solicitada = false;
	for (size_t i = 0; i < 1024; i++)
		keys_pressed[i] = false;
}

GLfloat Window::getXChange()
{
	GLfloat the_change = x_change;
	x_change = 0.0f;
	return the_change;
}

GLfloat Window::getYChange()
{
	GLfloat the_change = y_change;
	y_change = 0.0f;
	return the_change;
}

//>>>>>>>>>>>>>>>>>>>>>> Eventos de teclado >>>>>>>>>>>>>>>>>>>>>>
void Window::ManejaTeclado(GLFWwindow* window, int key, int, int action, int)
{
	Window* the_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
    {
        //>>>>>>>>>>>>>>>>>>>>>> Seleccion de escena >>>>>>>>>>>>>>>>>>>>>>>>
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4)
            the_window->escena_activa = key - GLFW_KEY_1;
        else if (key >= GLFW_KEY_KP_1 && key <= GLFW_KEY_KP_4)
            the_window->escena_activa = key - GLFW_KEY_KP_1;

        //>>>>>>>>>>>>>>>>>>>>>> Controles de la nave >>>>>>>>>>>>>>>>>>>>>>>
        if (the_window->escena_activa == 1)
        {
            if (key == GLFW_KEY_Y) the_window->luz_nave_encendida = !the_window->luz_nave_encendida;
            if (key == GLFW_KEY_U) the_window->motor1_encendido = !the_window->motor1_encendido;
            if (key == GLFW_KEY_I) the_window->motor2_encendido = !the_window->motor2_encendido;
            if (key == GLFW_KEY_O) the_window->vista_general_solicitada = true;
        }
    }

    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9)
        key = GLFW_KEY_0 + (key - GLFW_KEY_KP_0);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			the_window->keys[key] = true;
			the_window->keys_pressed[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			the_window->keys[key] = false;
		}
	}
}

//>>>>>>>>>>>>>>>>>>>>>> Eventos del raton >>>>>>>>>>>>>>>>>>>>>>
void Window::ManejaMouse(GLFWwindow* window, double x_pos, double y_pos)
{
	Window* the_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (the_window->mouse_first_moved)
	{
		the_window->last_x = (GLfloat)x_pos;
		the_window->last_y = (GLfloat)y_pos;
		the_window->mouse_first_moved = false;
	}

	the_window->x_change = (GLfloat)x_pos - the_window->last_x;
	the_window->y_change = the_window->last_y - (GLfloat)y_pos;

	the_window->last_x = (GLfloat)x_pos;
	the_window->last_y = (GLfloat)y_pos;
}

//>>>>>>>>>>>>>>>>>>>>>> Cambio de dimensiones >>>>>>>>>>>>>>>>>>>>>>
void Window::ManejaResize(GLFWwindow* window, int w, int h)
{
	Window* the_window = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (w == 0 || h == 0) return;   // ventana minimizada
	the_window->buffer_width = w;
	the_window->buffer_height = h;
	glViewport(0, 0, w, h);
}

//>>>>>>>>>>>>>>>>>>>>>> Liberacion de la ventana >>>>>>>>>>>>>>>>>>>>>>
Window::~Window()
{
	if (main_window)
		glfwDestroyWindow(main_window);
	glfwTerminate();
}
