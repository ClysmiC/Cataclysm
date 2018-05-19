#pragma once

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "Types.h"

struct Window
{
	GLFWwindow* glfwWindow;
	int init(uint32 width, uint32 height);

	// TODO: make these functions that request dimensions from
	// glfwGetFramebufferSize(window, &w, &h);
	
	// uint32 height;
	// uint32 width;

private:
	// Callbacks the window registers with GLFW
	static void glfwWindowResizeCallback(GLFWwindow* window, int width, int height);
	static void glfwKeyPressedCallback(GLFWwindow* window, int key, int scanCode, int action, int mode);
	static void glfwMouseMovedCallback(GLFWwindow* window, double xPos, double yPos);
	static void glfwErrorCallback(int e, const char* message);
};

