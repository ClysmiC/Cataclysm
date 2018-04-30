#include "Game.h"

#include "Types.h"
#include "Window.h"

#include <thread>

#include "ResourceManager.h"
#include "Mesh.h"
#include "GL/glew.h"

#include "PointLight.h"

bool keys[1024];
bool lastKeys[1024];

int WinMain()
{
	// INIT WINDOW
	uint32 windowWidth = 1024;
	uint32 windowHeight = 720;
	
	Window window;

	if(window.init(windowWidth, windowHeight) < 0)
	{
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	ResourceManager::instance().init();
	// Mesh *nano = ResourceManager::instance().initMesh("nanosuit/nanosuit.obj", true, true);
    // Mesh *cyborg = ResourceManager::instance().initMesh("cyborg/cyborg.obj", true, true);
    // Mesh *bulb = ResourceManager::instance().initMesh("bulb/bulb.obj", false, true);

	PointLight* light = new PointLight();
	
	while(!glfwWindowShouldClose(window.glfwWindow))
	{
		glfwPollEvents();
		glClearColor(.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto v = glGetError();

		float secondsElapsed = glfwGetTime();

		Quaternion rotation = axisAngle(Vec3(0, 1, 0), -180 * sinf(secondsElapsed / 5));
		Quaternion bulbRotation = axisAngle(Vec3(0, 1, 0), 30 * sinf(secondsElapsed));

		light->position = Vec3(0, 0, -2);
		light->draw();

		glfwSwapBuffers(window.glfwWindow);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	glfwTerminate();
	return 0;
}
