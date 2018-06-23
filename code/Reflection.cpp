#include "Reflection.h"
#include "imgui/imgui.h"

#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"
#include "PortalComponent.h"
#include "RenderComponent.h"

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

void reflectVec3Rgb(IReflector* reflector, uint32 startingOffset)
{
    reflector->consumeFloat32("R", startingOffset + offsetof(Vec3, x));
    reflector->consumeFloat32("G", startingOffset + offsetof(Vec3, y));
    reflector->consumeFloat32("B", startingOffset + offsetof(Vec3, z));
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

void reflectTransformComponent(IReflector* reflector, uint32 startingOffset)
{
    if (reflector->pushStruct("Position"))
    {
        reflectVec3(reflector, startingOffset + offsetof(TransformComponent, position));
        reflector->popStruct();
    }

    if (reflector->pushStruct("Orientation"))
    {
        reflectQuaternion(reflector, startingOffset + offsetof(TransformComponent, orientation));
        reflector->popStruct();
    }

    if (reflector->pushStruct("Scale"))
    {
        reflectVec3(reflector, startingOffset + offsetof(TransformComponent, scale));
        reflector->popStruct();
    }
}

void reflectColliderComponent(IReflector* reflector, ColliderComponent* collider, uint32 startingOffset)
{
    // TODO: change this to consume enum when implemented
    reflector->consumeInt32("Type", startingOffset + offsetof(ColliderComponent, type));

    if (reflector->pushStruct("Offset"))
    {
        reflectVec3(reflector, startingOffset + offsetof(ColliderComponent, xfmOffset));
        reflector->popStruct();
    }
    
    if (collider->type == ColliderType::RECT3)
    {
        reflector->consumeFloat32("X Len", startingOffset + offsetof(ColliderComponent, xLength));
        reflector->consumeFloat32("Y Len", startingOffset + offsetof(ColliderComponent, yLength));
        reflector->consumeFloat32("Z Len", startingOffset + offsetof(ColliderComponent, zLength));
    }
    else // sphere, cylinder, capsule 
    {
        if (collider->type != ColliderType::SPHERE)
        {
            reflector->consumeFloat32("Length", startingOffset + offsetof(ColliderComponent, length));
        }
        
        reflector->consumeFloat32("Radius", startingOffset + offsetof(ColliderComponent, radius));

        // todo change this to enum
        reflector->consumeInt32("Axis", startingOffset + offsetof(ColliderComponent, axis));
    }
}

void reflectCameraComponent(IReflector* reflector, CameraComponent* camera, uint32 startingOffset)
{
    reflector->consumeBool("Is Ortho?", startingOffset + offsetof(CameraComponent, isOrthographic));
    
    if (camera->isOrthographic)
    {
        reflector->consumeFloat32("Width", startingOffset + offsetof(CameraComponent, orthoWidth));
    }
    else
    {
        reflector->consumeFloat32("FOV", startingOffset + offsetof(CameraComponent, perspectiveFov));
    }

    reflector->consumeFloat32("Aspect Ratio", startingOffset + offsetof(CameraComponent, aspectRatio));
    reflector->consumeFloat32("Near", startingOffset + offsetof(CameraComponent, near));
    reflector->consumeFloat32("Far", startingOffset + offsetof(CameraComponent, far));
}

void reflectDirectionalLightComponent(IReflector* reflector, uint32 startingOffset)
{
    if (reflector->pushStruct("Intensity"))
    {
        reflectVec3Rgb(reflector, startingOffset + offsetof(DirectionalLightComponent, intensity));
        reflector->popStruct();
    }

    if (reflector->pushStruct("Direction"))
    {
        reflectVec3(reflector, startingOffset + offsetof(DirectionalLightComponent, direction));
        reflector->popStruct();
    }
}

void reflectPointLightComponent(IReflector* reflector, uint32 startingOffset)
{
    if (reflector->pushStruct("Intensity"))
    {
        reflectVec3Rgb(reflector, startingOffset + offsetof(PointLightComponent, intensity));
        reflector->popStruct();
    }

    reflector->consumeFloat32("Attenuation Constant", startingOffset + offsetof(PointLightComponent, attenuationConstant));
    reflector->consumeFloat32("Attenuation Linear", startingOffset + offsetof(PointLightComponent, attenuationLinear));
    reflector->consumeFloat32("Attenuation Quadratic", startingOffset + offsetof(PointLightComponent, attenuationQuadratic));
}

void reflectPortalComponent(IReflector* reflector, uint32 startingOffset)
{
    // nothing to reflect for ui ?
}

void reflectRenderComponent(IReflector* reflector, uint32 startingOffset)
{
    // todo
}

IReflector::IReflector(ReflectionPurpose purpose)
    : purpose(purpose)
{
}

UiReflector::UiReflector()
    : IReflector(ReflectionPurpose::UI)
{
}

void UiReflector::startReflection(std::string label)
{
    bool someBool;
    ImGui::Begin(("Entity: " + label).c_str(), &someBool);
}

void UiReflector::endReflection()
{
    ImGui::End();
}

bool UiReflector::pushStruct(std::string name)
{
    bool result = ImGui::TreeNode(name.c_str());
    if (result)
    {
        this->indentationLevel++;
    }

    return result;
}

void UiReflector::popStruct()
{
    ImGui::TreePop();
    this->indentationLevel--;
}

void UiReflector::consumeInt32(std::string name, uint32 offset)
{
    int32* valuePtr = (int32*)((char*)reflectionTarget + offset);
    int32 value = *valuePtr;

    std::string text = name + ": " + std::to_string(value);

    ImGui::Text(text.c_str());
}

void UiReflector::consumeUInt32(std::string name, uint32 offset)
{
    uint32* valuePtr = (uint32*)((char*)reflectionTarget + offset);
    uint32 value = *valuePtr;

    std::string text = name + ": " + std::to_string(value);
    
    ImGui::Text(text.c_str());
}

void UiReflector::consumeFloat32(std::string name, uint32 offset)
{
    float32* valuePtr = (float32*)((char*)reflectionTarget + offset);
    float32 valueCopy = *valuePtr;

    typedef ImGuiInputTextFlags_ Flags;
    
    uint32 flags = Flags::ImGuiInputTextFlags_CharsDecimal | Flags::ImGuiInputTextFlags_AutoSelectAll | Flags::ImGuiInputTextFlags_EnterReturnsTrue;
        
    if (ImGui::InputScalar(name.c_str(), ImGuiDataType_Float, &valueCopy))
    {
        // This updates the actual value
        *valuePtr = valueCopy;
    }
}

void UiReflector::consumeFloat64(std::string name, uint32 offset)
{
    float64* valuePtr = (float64*)((char*)reflectionTarget + offset);
    float64 value = *valuePtr;

    std::string text = name + ": " + std::to_string(value);
    
    ImGui::Text(text.c_str());
}

void UiReflector::consumeBool(std::string name, uint32 offset)
{
    bool* valuePtr = (bool*)((char*)reflectionTarget + offset);
    bool value = *valuePtr;

    std::string text = name + ": " + std::to_string(value);
    
    ImGui::Text(text.c_str());
}

void UiReflector::consumeEnum(std::string name, uint32 offset)
{
    // TODO: not sure how this should work...
    
    // float32* valuePtr = (float32*)((char*)reflectionTarget + offset);
    // float32 value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);
    
    // ImGui::Text(text.c_str());
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
    UiReflector reflector;
    reflector.startReflection(e.friendlyName);
    
    TransformComponent* transform = getTransformComponent(e);
    if (transform)
    {
        if (ImGui::CollapsingHeader("Transform"))
        {
            reflector.reflectionTarget = transform;
            reflectTransformComponent(&reflector, 0);
        }
    }
    
    ColliderComponent* collider = getColliderComponent(e);
    if (collider)
    {
        if (ImGui::CollapsingHeader("Collider"))
        {
            reflector.reflectionTarget = collider;
            reflectColliderComponent(&reflector, collider, 0);
        }
    }
    
    CameraComponent* camera = getCameraComponent(e);
    if (camera)
    {
        if (ImGui::CollapsingHeader("Camera"))
        {
            reflector.reflectionTarget = camera;
            reflectCameraComponent(&reflector, camera, 0);
        }
    }
    
    DirectionalLightComponent* directionalLight = getDirectionalLightComponent(e);
    if (directionalLight)
    {
        if (ImGui::CollapsingHeader("Directional Light"))
        {
            reflector.reflectionTarget = directionalLight;
            reflectDirectionalLightComponent(&reflector, 0);
        }
    }
    
    PointLightComponent* pointLight = getPointLightComponent(e);
    if (pointLight)
    {
        if (ImGui::CollapsingHeader("Point Light"))
        {
            reflector.reflectionTarget = pointLight;
            reflectPointLightComponent(&reflector, 0);
        }
    }
    
    PortalComponent* portal = getPortalComponent(e);
    if (portal)
    {
        if (ImGui::CollapsingHeader("Portal"))
        {
            reflector.reflectionTarget = portal;
            reflectPortalComponent(&reflector, 0);
        }
    }
    
    // TODO
    // RenderComponent* t = getRenderComponent(e);
    

    reflector.endReflection();
}
