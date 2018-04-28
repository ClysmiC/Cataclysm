#pragma once

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "als_type.h"

struct Window
{
	GLFWwindow* glfwWindow;
	int init(uint32 width, uint32 height);

private:
	// Callbacks the window registers with GLFW
	static void glfwWindowResizeCallback(GLFWwindow* window, int width, int height);
	static void glfwKeyPressedCallback(GLFWwindow* window, int key, int scanCode, int action, int mode);
	static void glfwErrorCallback(int e, const char* message);
};

