#include "PointLight.h"
#include "ResourceManager.h"


PointLight::PointLight()
{
    ResourceManager& rm = ResourceManager::instance();

    rm.initMesh("bulb/bulb.obj", false, true);
    rm.initMaterial("", "bulbMaterial", true);
    rm.initShader("shader/light.vert", "shader/light.frag", true);

    mesh = rm.getMesh("bulb/bulb.obj");
    mesh->submeshes[1].material = rm.getMaterial("", "bulbMaterial");
    mesh->submeshes[1].material->shader = rm.getShader("shader/light.vert", "shader/light.frag");
    mesh->submeshes[1].material->clearUniforms();
	mesh->submeshes[1].material->vec3Uniforms.emplace("lightColor", Vec3(1, 1, 1));
}


PointLight::~PointLight()
{
}
