#include "Game.h"
#include "Window.h"

#include <string>
#include <fstream>

void Window::glfwErrorCallback(int e, const char* message)
{
	printf("%s\n", message);
}
	
void Window::glfwKeyPressedCallback(GLFWwindow* window, int key, int scanCode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (action == GLFW_PRESS)
	{
		keys[key] = true;
	}

	if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}
}

void Window::glfwMouseMovedCallback(GLFWwindow* window, double xPos, double yPos)
{
	mouseX = (float)xPos;
	mouseY = (float)yPos;
}

void Window::glfwWindowResizeCallback(GLFWwindow* window, int w, int h)
{
	glfwGetFramebufferSize(window, &w, &h);
	
	glViewport(0, 0, w, h);

	// TODO: recalculate the perspective matrix!
}

int Window::init(uint32 width, uint32 height)
{
	// Initialize GLFW, create and open window with OpenGL context
	if (!glfwInit())
	{
		printf("Failed to initialize GLFW. Exiting.\n");
		return -1;
	}

	mouseXPrev = FLT_MAX;
	mouseYPrev = FLT_MAX;

	glfwSetErrorCallback(&glfwErrorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	
	glfwWindow = glfwCreateWindow(width, height, "Game", NULL, NULL);
	glfwMakeContextCurrent(glfwWindow);

	glViewport(0, 0, width, height);
	
	glfwSetKeyCallback(glfwWindow, &glfwKeyPressedCallback);
	glfwSetWindowSizeCallback(glfwWindow, &glfwWindowResizeCallback);

	glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(glfwWindow, &glfwMouseMovedCallback);
	
	glewExperimental = GL_TRUE;
	GLenum glewInitStatus = glewInit();

	// GLEW
	if(glewInitStatus != GLEW_OK)
	{
		printf("Failed to initialize GLEW. Exiting.\n");
		return -1;
	}

	// OpenGL setup
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Not sure how to reasonably implement proper diffuse shading for 2-sided
	// polygon... for now I will just cull back faces and if I need polygon to
	// be 2-sided, I'll draw two polygons with different winding orders.
	glEnable(GL_CULL_FACE);

	return 0;
}

