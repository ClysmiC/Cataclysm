#include "Game.h"

#include "Types.h"
#include "Window.h"

#include <thread>

#include "ResourceManager.h"
#include "Mesh.h"
#include "GL/glew.h"

bool keys[1024];
bool lastKeys[1024];
real32 deltaTMs;


void updateCamera(Camera& camera)
{
	real32 cameraTurnSpeed = 30; // Deg / Sec
	real32 cameraSpeed = 1;
	real32 deltaTS = deltaTMs / 1000.0f;
	
	// if (keys[GLFW_KEY_W])
	// {
	// 	camera.position += camera.forward() * cameraSpeed * deltaTS;
	// }
	// else if (keys[GLFW_KEY_S])
	// {
	// 	camera.position += -camera.forward() * cameraSpeed * deltaTS;
	// }
		
	// if (keys[GLFW_KEY_A])
	// {
	// 	camera.position += -camera.right() * cameraSpeed * deltaTS;
	// }
	// else if (keys[GLFW_KEY_D])
	// {
	// 	camera.position += camera.right() * cameraSpeed * deltaTS;
	// }

	// if (keys[GLFW_KEY_Q])
	// {
	// 	Quaternion turn = axisAngle(Vec3(0, 1, 0), cameraTurnSpeed * deltaTS);
	// 	camera.orientation = camera.orientation * turn;
	// }
	// else if (keys[GLFW_KEY_E])
	// {
	// 	Quaternion turn = axisAngle(Vec3(0, 1, 0), -cameraTurnSpeed * deltaTS);
	// 	camera.orientation = camera.orientation * turn;
	// }	
}


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

	Camera camera;

	real32 lastTimeMs = 0;
	
	while(!glfwWindowShouldClose(window.glfwWindow))
	{
		glfwPollEvents();
		glClearColor(.5f, 0.5f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto v = glGetError();

		real32 timeS = glfwGetTime();
		real32 timeMs = timeS * 1000.0f;
		deltaTMs = timeMs - lastTimeMs;
		lastTimeMs = timeMs;

		updateCamera(camera);

		Quaternion rotation = axisAngle(Vec3(0, 1, 0), -180 * sinf(timeS / 5));
		Quaternion bulbRotation = axisAngle(Vec3(0, 1, 0), 30 * sinf(timeS));

		// light->position = Vec3(0, 0, -2);
		// light->draw(camera);

		glfwSwapBuffers(window.glfwWindow);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	glfwTerminate();
	return 0;
}
