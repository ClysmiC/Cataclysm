#include "Game.h"
#include "Window.h"

#include <string>
#include <fstream>

void glfwErrorCallback(int e, const char* message)
{
	printf("%s\n", message);
}
	
void glfwKeyPressedCallback(GLFWwindow* window, int key, int scanCode, int action, int modifiers)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (action == GLFW_PRESS)
	{
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}
}

void glfwMousePressedCallback(GLFWwindow* window, int button, int action, int modifiers)
{
	if (action == GLFW_PRESS)
	{
		mouseButtons[button] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		mouseButtons[button] = false;
	}
}

void glfwMouseMovedCallback(GLFWwindow* window, double xPos, double yPos)
{
	mouseX = (float)xPos;
	mouseY = (float)yPos;
}

void glfwWindowResizeCallback(GLFWwindow* window, int w, int h)
{
	glfwGetFramebufferSize(window, &w, &h);
	
	glViewport(0, 0, w, h);

	// TODO: recalculate the perspective matrix?
}

bool initGlfwWindow(Window* window, uint32 width, uint32 height)
{
	// Initialize GLFW, create and open window with OpenGL context
	if (!glfwInit())
	{
		printf("Failed to initialize GLFW. Exiting.\n");
		return false;
	}

	mouseXPrev = FLT_MAX;
	mouseYPrev = FLT_MAX;

	glfwSetErrorCallback(glfwErrorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	
	window->glfwWindow = glfwCreateWindow(width, height, "Game", NULL, NULL);
	glfwMakeContextCurrent(window->glfwWindow);

	glViewport(0, 0, width, height);
	
	glfwSetKeyCallback(window->glfwWindow, glfwKeyPressedCallback);
	glfwSetMouseButtonCallback(window->glfwWindow, glfwMousePressedCallback);
	glfwSetWindowSizeCallback(window->glfwWindow, glfwWindowResizeCallback);

	glfwSetInputMode(window->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window->glfwWindow, glfwMouseMovedCallback);
	
	glewExperimental = GL_TRUE;
	GLenum glewInitStatus = glewInit();

	// GLEW
	if(glewInitStatus != GLEW_OK)
	{
		printf("Failed to initialize GLEW. Exiting.\n");
		return false;
	}

	// OpenGL setup
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Not sure how to reasonably implement proper diffuse shading for 2-sided
	// polygon... for now I will just cull back faces and if I need polygon to
	// be 2-sided, I'll draw two polygons with different winding orders.
	glEnable(GL_CULL_FACE);

	return true;
}

