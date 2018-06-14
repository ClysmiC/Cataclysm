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

Transform* getSourceSceneXfm(PortalComponent* portal, Scene* sourceScene)
{
	if (portal->scene1 == sourceScene) return &portal->scene1Xfm;
    if (portal->scene2 == sourceScene) return &portal->scene2Xfm;

	return nullptr;
}

Transform* getDestSceneXfm(PortalComponent* portal, Scene* sourceScene)
{
	if (portal->scene1 == sourceScene) return &portal->scene2Xfm;
	if (portal->scene2 == sourceScene) return &portal->scene1Xfm;

	return nullptr;
}

Vec3 intoSourcePortalNormal(PortalComponent* portal, Scene* sourceScene)
{
	Transform* xfm = getSourceSceneXfm(portal, sourceScene);
	Vec3 result = xfm->back();
	return result;
}

Vec3 outOfSourcePortalNormal(PortalComponent* portal, Scene* sourceScene)
{
	Transform* xfm = getSourceSceneXfm(portal, sourceScene);
	Vec3 result = xfm->forward();
	return result;
}

Vec3 intoDestPortalNormal(PortalComponent* portal, Scene* sourceScene)
{
	Transform* xfm = getDestSceneXfm(portal, sourceScene);
	Vec3 result = xfm->back();
	return result;
}

Vec3 outOfDestPortalNormal(PortalComponent* portal, Scene* sourceScene)
{
	Transform* xfm = getDestSceneXfm(portal, sourceScene);
	Vec3 result = xfm->forward();
	return result;
}

void rebaseTransformInPlace(PortalComponent* portal, Scene* sourceScene, Transform* transform)
{
	Transform* sourceSceneXfm = getSourceSceneXfm(portal, sourceScene);
	Transform* destSceneXfm = getDestSceneXfm(portal, sourceScene);

	Quaternion intoSourcePortal = axisAngle(sourceSceneXfm->up(), 180) * sourceSceneXfm->orientation;
	Quaternion outOfDestPortal = destSceneXfm->orientation;
	
	Quaternion rotationNeeded = relativeRotation(intoSourcePortal, outOfDestPortal);
	
	Vec3 offsetToSourcePortal = sourceSceneXfm->position - transform->position;
	Vec3 offsetToSourcePortalTransformedToDestScene = rotationNeeded * offsetToSourcePortal;

	Vec3 positionInDestScene = destSceneXfm->position - offsetToSourcePortalTransformedToDestScene; 

	transform->position = positionInDestScene;
	transform->orientation = rotationNeeded * transform->orientation;
}
