#include "Game.h"

#include "Types.h"
#include "Window.h"

#include <thread>

#include "Ecs.h"
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
	Mesh *nano = ResourceManager::instance().initMesh("nanosuit/nanosuit.obj", true, true);
	Mesh *bulb = ResourceManager::instance().initMesh("bulb/bulb.obj", false, true);
	
    // Mesh *cyborg = ResourceManager::instance().initMesh("cyborg/cyborg.obj", true, true);

	Camera camera;

	real32 lastTimeMs = 0;
	
    Ecs ecs;
    Entity test = ecs.nextEntityId();
    // Set up test mesh
    {
        Mesh* m = nano;
        
        TransformComponent *tc = ecs.addTransformComponent(test);
        tc->setPosition(0, -2, -4);
		tc->setScale(0.25);
		
        RenderComponentCollection rcc = ecs.addRenderComponents(test, m->submeshes.size());

        for(uint32 i = 0; i < rcc.numComponents; i++)
        {
            RenderComponent *rc = rcc.renderComponents + i;
            rc->init(m->submeshes[i]);
        }
    }
	
	Entity light = ecs.nextEntityId();
	// Set up light
	{
		ResourceManager& rm = ResourceManager::instance();
		
        Mesh* m = bulb;
        
        TransformComponent *tc = ecs.addTransformComponent(light);
        tc->setPosition(-2, 0, -7);
        tc->setScale(.35);
		
        RenderComponentCollection rcc = ecs.addRenderComponents(light, m->submeshes.size());

        for(uint32 i = 0; i < rcc.numComponents; i++)
        {
            RenderComponent *rc = rcc.renderComponents + i;
            rc->init(m->submeshes[i]);
        }

		PointLightComponent *plc = ecs.addPointLightComponent(light);
		plc->intensity = Vec3(5, 5, 5);

		rm.initMaterial("", "bulbMaterial", true);
		rm.initShader("shader/light.vert", "shader/light.frag", true);

		m->submeshes[1].material = rm.getMaterial("", "bulbMaterial");
		m->submeshes[1].material->receiveLight = false;
		m->submeshes[1].material->shader = rm.getShader("shader/light.vert", "shader/light.frag");
		m->submeshes[1].material->clearUniforms();
		m->submeshes[1].material->vec3Uniforms.emplace("lightColor", Vec3(1, 1, 1));
	}

	TransformComponent *lightXfm = ecs.getTransformComponent(light);
    TransformComponent *testXfm = ecs.getTransformComponent(test);

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
		
		Quaternion lightRot = axisAngle(Vec3(0, 1, 0), 30 * timeS);
        testXfm->setOrientation(lightRot);
		
		Mat4 xfm;
		xfm.rotateInPlace(lightRot);
		xfm.translateInPlace(Vec3(0, 0, -1));

		Vec4 rotatePosDelta = xfm * Vec4(0, 0, 0, 1);
		lightXfm->setPosition(Vec3(0, 0, -4) + Vec3(rotatePosDelta.x, 0, rotatePosDelta.z));

        ecs.renderAllRenderComponents(camera);

		glfwSwapBuffers(window.glfwWindow);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	glfwTerminate();
	return 0;
}
