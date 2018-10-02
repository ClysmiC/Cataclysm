#include "resource/resources/Texture.h"
#include "GL/glew.h"

struct Ecs;
struct Scene;
struct CameraComponent;
struct ITransform;
struct PointLightComponent;

struct Renderer
{
    GLuint shadowMapFbo;
    TextureData shadowMap;
};

void initRenderer(Renderer* renderer);

PointLightComponent* closestPointLight(Ecs* ecs, ITransform* xfm);
void renderAllRenderComponents(Renderer* renderer, Ecs* ecs, CameraComponent* camera, ITransform* cameraXfm, bool renderingViaPortal=false, ITransform* destPortalXfm=nullptr);
void renderContentsOfAllPortals(Renderer* renderer, Scene* scene, CameraComponent* camera, ITransform* cameraXfm, uint32 recursionLevel=0);
