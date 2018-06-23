#include "Reflection.h"
#include "imgui/imgui.h"

void reflectVec2(IReflector* reflector, uint32 startingOffset)
{
    reflector->consumeFloat32("X", startingOffset + offsetof(Vec2, x));
    reflector->consumeFloat32("Y", startingOffset + offsetof(Vec2, y));
}

void reflectVec3(IReflector* reflector, uint32 startingOffset)
{
    reflector->consumeFloat32("X", startingOffset + offsetof(Vec3, x));
    reflector->consumeFloat32("Y", startingOffset + offsetof(Vec3, y));
    reflector->consumeFloat32("Z", startingOffset + offsetof(Vec3, z));
}

void reflectVec4(IReflector* reflector, uint32 startingOffset)
{
    reflector->consumeFloat32("X", startingOffset + offsetof(Vec4, x));
    reflector->consumeFloat32("Y", startingOffset + offsetof(Vec4, y));
    reflector->consumeFloat32("Z", startingOffset + offsetof(Vec4, z));
    reflector->consumeFloat32("W", startingOffset + offsetof(Vec4, w));
}

void reflectQuaternion(IReflector* reflector, uint32 startingOffset)
{
    reflector->consumeFloat32("X", startingOffset + offsetof(Quaternion, x));
    reflector->consumeFloat32("Y", startingOffset + offsetof(Quaternion, y));
    reflector->consumeFloat32("Z", startingOffset + offsetof(Quaternion, z));
    reflector->consumeFloat32("W", startingOffset + offsetof(Quaternion, w));
}

void UiReflector::pushStruct(std::string name)
{
    std::string text;
    for (uint32 i = 0; i < this->tabLevel; i++)
    {
        text += "\t";
    }
    text += name;
    
    ImGui::Text(text.c_str());
    this->tabLevel++;
}

void UiReflector::popStruct()
{
    this->tabLevel--;
}

void UiReflector::consumeFloat32(std::string name, uint32 offset)
{
    real32* valuePtr = (real32*)((char*)reflectionTarget + offset);
    real32 value = *valuePtr;

    std::string text;
    for (uint32 i = 0; i < this->tabLevel; i++)
    {
        text += "\t";
    }
    text += name + ": " + std::to_string(value);
    
    ImGui::Text(text.c_str());
}

#include "Entity.h"
#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"
#include "PortalComponent.h"
#include "Ecs.h"
void testUiReflection(Entity e)
{
    // TODO: handle component groups
    bool someBool;
    ImGui::Begin(("Entity: " + e.friendlyName).c_str(), &someBool);
    
    TransformComponent* transform = getTransformComponent(e);
    if (transform)
    {
        UiReflector reflector;
        reflector.reflectionTarget = transform;
        reflector.purpose = ReflectionPurpose::UI;

        reflectTransformComponent(&reflector, 0);
    }
    
    // ColliderComponent* collider = getColliderComponent(e);
    // CameraComponent* camera = getCameraComponent(e);
    // DirectionalLightComponent* directionalLight = getDirectionalLightComponent(e);
    // PointLightComponent* pointLight = getPointLightComponent(e);
    // PortalComponent* portal = getPortalComponent(e);
    
    // TODO
    // RenderComponent* t = getRenderComponent(e);
    
    ImGui::End();
}
