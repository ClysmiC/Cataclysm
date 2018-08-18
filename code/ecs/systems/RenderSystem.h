struct Ecs;
struct Scene;
struct CameraComponent;
struct ITransform;
struct PointLightComponent;

PointLightComponent* closestPointLight(Ecs* ecs, ITransform* xfm);
void renderAllRenderComponents(Ecs* ecs, CameraComponent* camera, ITransform* cameraXfm, bool renderingViaPortal=false, ITransform* destPortalXfm=nullptr);
void renderContentsOfAllPortals(Scene* scene, CameraComponent* camera, ITransform* cameraXfm, uint32 recursionLevel=0);
