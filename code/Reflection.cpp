#include "Reflection.h"
#include "imgui/imgui.h"

#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"
#include "PortalComponent.h"
#include "RenderComponent.h"

#include <cmath>

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
    ColliderComponent origCc = *collider;
    reflector->consumeEnum("Type", startingOffset + offsetof(ColliderComponent, type), ColliderTypeNames, (uint32)ColliderType::ENUM_VALUE_COUNT);

    //
    // If type was toggled, try to replace some of the union values with sane estimates based on the
    // collider before it was changed.
    //
    {
        if (origCc.type == ColliderType::RECT3 && collider->type != ColliderType::RECT3)
        {
            if (collider->type == ColliderType::SPHERE)
            {
                collider->radius = fmax(fmax(origCc.rect3Lengths.x, origCc.rect3Lengths.y), origCc.rect3Lengths.z);
            }
            else if (collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE)
            {
                if (origCc.rect3Lengths.x >= origCc.rect3Lengths.y && origCc.rect3Lengths.x >= origCc.rect3Lengths.z)
                {
                    collider->axis = Axis3D::X;
                    collider->length = origCc.rect3Lengths.x;
                    collider->radius = fmax(origCc.rect3Lengths.y, origCc.rect3Lengths.z);
                }
                else if (origCc.rect3Lengths.y >= origCc.rect3Lengths.x && origCc.rect3Lengths.y >= origCc.rect3Lengths.z)
                {
                    collider->axis = Axis3D::Y;
                    collider->length = origCc.rect3Lengths.y;
                    collider->radius = fmax(origCc.rect3Lengths.x, origCc.rect3Lengths.z);
                }
                else
                {
                    collider->axis = Axis3D::Z;
                    collider->length = origCc.rect3Lengths.z;
                    collider->radius = fmax(origCc.rect3Lengths.x, origCc.rect3Lengths.y);
                }
            }
            
        }
        else if (origCc.type != ColliderType::RECT3 && collider->type == ColliderType::RECT3)
        {
            collider->rect3Lengths.x = origCc.radius;
            collider->rect3Lengths.y = origCc.radius;
            collider->rect3Lengths.z = origCc.radius;
            
            if (origCc.type == ColliderType::CYLINDER || origCc.type == ColliderType::CAPSULE)
            {
                if (origCc.axis == Axis3D::X)
                {
                    collider->rect3Lengths.x = origCc.length;
                }
                else if (origCc.axis == Axis3D::Y)
                {
                    collider->rect3Lengths.y = origCc.length;
                }
                else // Z
                {
                    collider->rect3Lengths.z = origCc.length;
                }
            }
        }
    }

    if (reflector->pushStruct("Offset"))
    {
        reflectVec3(reflector, startingOffset + offsetof(ColliderComponent, xfmOffset));
        reflector->popStruct();
    }
    
    if (collider->type == ColliderType::RECT3)
    {
        if (reflector->pushStruct("Lengths"))
        {
            reflectVec3(reflector, startingOffset + offsetof(ColliderComponent, rect3Lengths));
            reflector->popStruct();
        }
    }
    else // sphere, cylinder, capsule 
    {
        if (collider->type != ColliderType::SPHERE)
        {
            reflector->consumeFloat32("Length", startingOffset + offsetof(ColliderComponent, length));
            reflector->consumeEnum("Axis", startingOffset + offsetof(ColliderComponent, axis), Axis3DNames, (uint32)Axis3D::ENUM_VALUE_COUNT);
        }
        
        reflector->consumeFloat32("Radius", startingOffset + offsetof(ColliderComponent, radius));
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

bool UiReflector::startReflection(std::string label)
{
    bool shouldClose;
    ImGui::Begin(("Entity: " + label).c_str(), &shouldClose);

    if (shouldClose)
    {
        return false;
    }

    return true;
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

int32 UiReflector::consumeInt32(std::string name, uint32 offset)
{
    int32* valuePtr = (int32*)((char*)reflectionTarget + offset);
    int32 value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);

    // ImGui::Text(text.c_str());

    return *valuePtr;
}

uint32 UiReflector::consumeUInt32(std::string name, uint32 offset)
{
    uint32* valuePtr = (uint32*)((char*)reflectionTarget + offset);
    uint32 value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);
    
    // ImGui::Text(text.c_str());

    return *valuePtr;
}

float32 UiReflector::consumeFloat32(std::string name, uint32 offset)
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

    return *valuePtr;
}

float64 UiReflector::consumeFloat64(std::string name, uint32 offset)
{
    float64* valuePtr = (float64*)((char*)reflectionTarget + offset);
    float64 value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);
    
    // ImGui::Text(text.c_str());

    return *valuePtr;
}

bool UiReflector::consumeBool(std::string name, uint32 offset)
{
    bool* valuePtr = (bool*)((char*)reflectionTarget + offset);
    bool value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);
    
    // ImGui::Text(text.c_str());

    return *valuePtr;
}

uint32 UiReflector::consumeEnum(std::string name, uint32 offset, std::string* enumNames, uint32 enumValueCount)
{
    // TODO: not sure how this should work...
    
    uint32* valuePtr = (uint32*)((char*)reflectionTarget + offset);
    uint32 value = *valuePtr;

    assert(value < enumValueCount);
    
    std::string* selectedValueName = enumNames + value;

    if (ImGui::TreeNode((name + ": " + *selectedValueName).c_str()))
    {
        for (uint32 i = 0; i < enumValueCount; i++)
        {
            bool selected = (i == value);
            std::string* valueName = enumNames + i;
            
            if (ImGui::Selectable(valueName->c_str(), selected))
            {
                // Update actual value
                *valuePtr = i;
            }
        }

        ImGui::TreePop();
    }

    return *valuePtr;
}

#include "Entity.h"
#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"
#include "PortalComponent.h"
#include "Ecs.h"
bool testUiReflection(Entity e)
{
    // TODO: handle component groups
    UiReflector reflector;
    if (!reflector.startReflection(e.friendlyName))
    {
        // todo, how to make this close ?
        // return false;
    }
    
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
    return true;
}
