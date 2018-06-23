#include "Game.h"
#include "Window.h"

#include <string>
#include <fstream>
#include <cmath>

void cata_glfwErrorCallback(int e, const char* message)
{
    printf("%s\n", message);
}
    
void cata_glfwKeyPressedCallback(GLFWwindow* window, int key, int scanCode, int action, int modifiers)
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

void cata_glfwMousePressedCallback(GLFWwindow* window, int button, int action, int modifiers)
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

void cata_glfwMouseMovedCallback(GLFWwindow* window, double xPos, double yPos)
{
    // TODO: querying this every frame might be slow depending on how it's implemented
    //       is there a way for us to directly set it on our Window struct?
    //       We would need a way to map from GLFWwindow to Window
    int32 w, h;
    glfwGetFramebufferSize(window, &w, &h);
    
    mouseX = (float)xPos;
    mouseY = ((float)-yPos) + h;
}

void cata_glfwWindowResizeCallback(GLFWwindow* window, int w, int h)
{
    // glfwGetFramebufferSize(window, &window->width, &window->height);
    
    // glViewport(0, 0, w, h);

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

    window->width = width;
    window->height = height;

    mouseXPrev = FLT_MAX;
    mouseYPrev = FLT_MAX;

    //glfwSetErrorCallback(glfwErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
    
    window->glfwWindow = glfwCreateWindow(width, height, "Game", NULL, NULL);
    glfwMakeContextCurrent(window->glfwWindow);

    glViewport(0, 0, width, height);
    
    // TODO: moved registering these to imgui_impl_glfw.cpp
    // TODO: unify this file and that file

    /*glfwSetKeyCallback(window->glfwWindow, glfwKeyPressedCallback);
    glfwSetMouseButtonCallback(window->glfwWindow, glfwMousePressedCallback);
    glfwSetWindowSizeCallback(window->glfwWindow, glfwWindowResizeCallback);
    glfwSetCursorPosCallback(window->glfwWindow, glfwMouseMovedCallback);*/

    glfwSetInputMode(window->glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
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

int32 windowWidth(Window* window)
{
    int32 w, h;
    glfwGetFramebufferSize(window->glfwWindow, &w, &h);
    return w;
}

int32 windowHeight(Window* window)
{
    int32 w, h;
    glfwGetFramebufferSize(window->glfwWindow, &w, &h);
    return h;
}

Vec2 pixelToViewportCoordinate(Window* window, Vec2 pixel)
{
    int32 w, h;
    glfwGetFramebufferSize(window->glfwWindow, &w, &h);

    Vec2 result;
    result.x = clamp(pixel.x / (float32)w, 0, 1);
    result.y = clamp(pixel.y / (float32)h, 0, 1);

    return result;
}

Vec2 viewportCoordinateToPixel(Window* window, Vec2 viewportCoordinate)
{
    int32 w, h;
    glfwGetFramebufferSize(window->glfwWindow, &w, &h);

    clamp(viewportCoordinate.x, 0, 1);
    clamp(viewportCoordinate.y, 0, 1);

    Vec2 result;
    result.x = std::floor(viewportCoordinate.x * w);
    result.y = std::floor(viewportCoordinate.y * h);

    if (result.x >= w - EPSILON) result.x = w - 1;
    if (result.y >= h - EPSILON) result.y = h - 1;

    return result;
}
