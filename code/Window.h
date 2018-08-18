#pragma once

#include <GL/glew.h>
#define GLFW_DLL
#include <GLFW/glfw3.h>

#include "als/als_types.h"
#include "als/als_math.h"

struct Window
{
    GLFWwindow* glfwWindow;

    // TODO: these get set at the start, but don't get updated
    // when the window is resized
    int32 height;
    int32 width;

    // Callbacks the window registers with GLFW
};

bool initGlfwWindow(Window* window, uint32 width, uint32 height);

int32 windowWidth(Window* window);
int32 windowHeight(Window* window);
Vec2 pixelToViewportCoordinate(Window* window, Vec2 pixel);
Vec2 viewportCoordinateToPixel(Window* window, Vec2 viewportCoordinate);

void cata_glfwWindowResizeCallback(GLFWwindow* window, int width, int height);
void cata_glfwKeyPressedCallback(GLFWwindow* window, int key, int scanCode, int action, int modifiers);
void cata_glfwMousePressedCallback(GLFWwindow* window, int button, int action, int modifiers);
void cata_glfwMouseMovedCallback(GLFWwindow* window, double xPos, double yPos);
void cata_glfwErrorCallback(int e, const char* message);
