#include "GL/glew.h"

#include "PortalComponent.h"
#include "ResourceManager.h"
#include "Ecs.h"

Shader* portalShader()
{
	static Shader* shader_ = nullptr;

	if (shader_ == nullptr)
	{
		shader_ = ResourceManager::instance().initShader("shader/portal.vert", "shader/portal.frag", true);
	}

	return shader_;
}

void setDimensions(PortalComponent* portal, Vec2 dimensions)
{
	Vec3 newScale(dimensions.x, dimensions.y, 1);
	portal->sourceSceneXfm.scale = newScale;
	portal->destSceneXfm.scale = newScale;
}

Vec2 getDimensions(PortalComponent* portal)
{
	Vec2 result = Vec2(portal->sourceSceneXfm.scale.x, portal->sourceSceneXfm.scale.y);

	return result;
}
