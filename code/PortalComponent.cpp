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
	portal->scene1Xfm.scale = newScale;
	portal->scene2Xfm.scale = newScale;
}

Vec2 getDimensions(PortalComponent* portal)
{
	Vec2 result = Vec2(portal->scene1Xfm.scale.x, portal->scene1Xfm.scale.y);

	return result;
}

Scene* getDestScene(PortalComponent* portal, Scene* sourceScene)
{
	if (portal->scene1 == sourceScene) return portal->scene2;
	if (portal->scene2 == sourceScene) return portal->scene1;

	return nullptr;
}

TransformComponent* getSourceSceneXfm(PortalComponent* portal, Scene* sourceScene)
{
	if (portal->scene1 == sourceScene) return &portal->scene1Xfm;
    if (portal->scene2 == sourceScene) return &portal->scene2Xfm;

	return nullptr;
}

TransformComponent* getDestSceneXfm(PortalComponent* portal, Scene* sourceScene)
{
	if (portal->scene1 == sourceScene) return &portal->scene2Xfm;
	if (portal->scene2 == sourceScene) return &portal->scene1Xfm;

	return nullptr;
}
