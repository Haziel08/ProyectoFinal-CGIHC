#include "Window.h"

//>>>>>>>>>>>>>>>>>>>>>> Constructores >>>>>>>>>>>>>>>>>>>>>>
Window::Window() : Window(1280, 720) {}

Window::Window(GLint windowWidth, GLint windowHeight)
{
    escenaActiva = 0;
    luzNaveEncendida = true;
    motor1Encendido = true;
    motor2Encendido = true;
    vistaGeneralSolicitada = false;
	mainWindow = nullptr;
	width = windowWidth;
	height = windowHeight;
	bufferWidth = windowWidth;
	bufferHeight = windowHeight;

	lastX = lastY = 0.0f;
	xChange = 0.0f;
	yChange = 0.0f;
	mouseFirstMoved = true;

	for (size_t i = 0; i < 1024; i++)
	{
		keys[i] = false;
		keysPressed[i] = false;
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

	mainWindow = glfwCreateWindow(width, height, titulo, NULL, NULL);
	if (!mainWindow)
	{
		printf("Fallo al crear la ventana con GLFW\n");
		glfwTerminate();
		return 1;
	}

	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
	glfwMakeContextCurrent(mainWindow);
	createCallbacks();

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		printf("Fallo inicializacion de GLEW\n");
		glfwDestroyWindow(mainWindow);
		mainWindow = nullptr;
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, bufferWidth, bufferHeight);
	glfwSetWindowUserPointer(mainWindow, this);
	return 0;
}

//>>>>>>>>>>>>>>>>>>>>>> Registro de eventos >>>>>>>>>>>>>>>>>>>>>>
void Window::createCallbacks()
{
	glfwSetKeyCallback(mainWindow, ManejaTeclado);
	glfwSetCursorPosCallback(mainWindow, ManejaMouse);
	glfwSetFramebufferSizeCallback(mainWindow, ManejaResize);
}

bool Window::TeclaSostenida(int key) const
{
	return key >= 0 && key < 1024 && keys[key];
}

bool Window::TeclaPresionada(int key) const
{
	return key >= 0 && key < 1024 && keysPressed[key];
}

void Window::NuevoFrame()
{
    vistaGeneralSolicitada = false;
	for (size_t i = 0; i < 1024; i++)
		keysPressed[i] = false;
}

GLfloat Window::getXChange()
{
	GLfloat theChange = xChange;
	xChange = 0.0f;
	return theChange;
}

GLfloat Window::getYChange()
{
	GLfloat theChange = yChange;
	yChange = 0.0f;
	return theChange;
}

//>>>>>>>>>>>>>>>>>>>>>> Eventos de teclado >>>>>>>>>>>>>>>>>>>>>>
void Window::ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
    {
        //>>>>>>>>>>>>>>>>>>>>>> Seleccion de escena >>>>>>>>>>>>>>>>>>>>>>>>
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_4)
            theWindow->escenaActiva = key - GLFW_KEY_1;

        //>>>>>>>>>>>>>>>>>>>>>> Controles de la nave >>>>>>>>>>>>>>>>>>>>>>>
        if (theWindow->escenaActiva == 1)
        {
            if (key == GLFW_KEY_Z) theWindow->luzNaveEncendida = !theWindow->luzNaveEncendida;
            if (key == GLFW_KEY_X) theWindow->motor1Encendido = !theWindow->motor1Encendido;
            if (key == GLFW_KEY_C) theWindow->motor2Encendido = !theWindow->motor2Encendido;
            if (key == GLFW_KEY_V) theWindow->vistaGeneralSolicitada = true;
        }
    }

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			theWindow->keys[key] = true;
			theWindow->keysPressed[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			theWindow->keys[key] = false;
		}
	}
}

//>>>>>>>>>>>>>>>>>>>>>> Eventos del raton >>>>>>>>>>>>>>>>>>>>>>
void Window::ManejaMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (theWindow->mouseFirstMoved)
	{
		theWindow->lastX = (GLfloat)xPos;
		theWindow->lastY = (GLfloat)yPos;
		theWindow->mouseFirstMoved = false;
	}

	theWindow->xChange = (GLfloat)xPos - theWindow->lastX;
	theWindow->yChange = theWindow->lastY - (GLfloat)yPos;

	theWindow->lastX = (GLfloat)xPos;
	theWindow->lastY = (GLfloat)yPos;
}

//>>>>>>>>>>>>>>>>>>>>>> Cambio de dimensiones >>>>>>>>>>>>>>>>>>>>>>
void Window::ManejaResize(GLFWwindow* window, int w, int h)
{
	Window* theWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	if (w == 0 || h == 0) return;   // ventana minimizada
	theWindow->bufferWidth = w;
	theWindow->bufferHeight = h;
	glViewport(0, 0, w, h);
}

//>>>>>>>>>>>>>>>>>>>>>> Liberacion de la ventana >>>>>>>>>>>>>>>>>>>>>>
Window::~Window()
{
	if (mainWindow)
		glfwDestroyWindow(mainWindow);
	glfwTerminate();
}
