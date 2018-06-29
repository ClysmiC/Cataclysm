#include "Reflection.h"
#include "imgui/imgui.h"

#include "TransformComponent.h"
#include "ColliderComponent.h"
#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"
#include "PortalComponent.h"
#include "RenderComponent.h"

#include "DebugDraw.h"

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
        //
        // Switching away from Rect3
        //
        if (origCc.type == ColliderType::RECT3 && collider->type != ColliderType::RECT3)
        {
            if (collider->type == ColliderType::SPHERE)
            {
                collider->radius = fmax(fmax(origCc.rect3Lengths.x, origCc.rect3Lengths.y), origCc.rect3Lengths.z) / 2.0;
            }
            else if (collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE)
            {
                if (origCc.rect3Lengths.x >= origCc.rect3Lengths.y && origCc.rect3Lengths.x >= origCc.rect3Lengths.z)
                {
                    collider->axis = Axis3D::X;
                    collider->length = origCc.rect3Lengths.x;
                    collider->radius = fmax(origCc.rect3Lengths.y, origCc.rect3Lengths.z) / 2.0;
                }
                else if (origCc.rect3Lengths.y >= origCc.rect3Lengths.x && origCc.rect3Lengths.y >= origCc.rect3Lengths.z)
                {
                    collider->axis = Axis3D::Y;
                    collider->length = origCc.rect3Lengths.y;
                    collider->radius = fmax(origCc.rect3Lengths.x, origCc.rect3Lengths.z) / 2.0;
                }
                else
                {
                    collider->axis = Axis3D::Z;
                    collider->length = origCc.rect3Lengths.z;
                    collider->radius = fmax(origCc.rect3Lengths.x, origCc.rect3Lengths.y) / 2.0;
                }
            }
            
        }
        //
        // Switching toward Rect3
        //
        else if (origCc.type != ColliderType::RECT3 && collider->type == ColliderType::RECT3)
        {
            collider->rect3Lengths.x = origCc.radius * 2;
            collider->rect3Lengths.y = origCc.radius * 2;
            collider->rect3Lengths.z = origCc.radius * 2;
            
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
        //
        // Switching away from sphere
        //
        else if (origCc.type == ColliderType::SPHERE && collider->type != ColliderType::SPHERE)
        {
            if (collider->type == ColliderType::CYLINDER || collider->type == ColliderType::CAPSULE)
            {
                collider->axis = Axis3D::Y;
                collider->length = origCc.radius * 2;
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

bool UiReflector::startReflection(EntityNameString label)
{
    bool shouldStayOpen = true;
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Appearing);
    ImGui::Begin((label + "###e").cstr(), &shouldStayOpen, ImGuiWindowFlags_NoCollapse);

    if (!shouldStayOpen)
    {
        return false;
    }

    return true;
}

void UiReflector::endReflection()
{
    ImGui::End();
}

bool UiReflector::pushStruct(StructNameString name)
{
    bool result = ImGui::TreeNode(name.cstr());
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

int32 UiReflector::consumeInt32(FieldNameString name, uint32 offset)
{
    int32* valuePtr = (int32*)((char*)reflectionTarget + offset);
    int32 value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);

    // ImGui::Text(text.c_str());

    return *valuePtr;
}

uint32 UiReflector::consumeUInt32(FieldNameString name, uint32 offset)
{
    uint32* valuePtr = (uint32*)((char*)reflectionTarget + offset);
    uint32 value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);
    
    // ImGui::Text(text.c_str());

    return *valuePtr;
}

float32 UiReflector::consumeFloat32(FieldNameString name, uint32 offset)
{
    float32* valuePtr = (float32*)((char*)reflectionTarget + offset);
    float32 valueCopy = *valuePtr;

    typedef ImGuiInputTextFlags_ Flags;
    
    uint32 flags = Flags::ImGuiInputTextFlags_CharsDecimal | Flags::ImGuiInputTextFlags_AutoSelectAll | Flags::ImGuiInputTextFlags_EnterReturnsTrue;
        
    if (ImGui::InputScalar(name.cstr(), ImGuiDataType_Float, &valueCopy))
    {
        // This updates the actual value
        *valuePtr = valueCopy;
    }

    return *valuePtr;
}

float64 UiReflector::consumeFloat64(FieldNameString name, uint32 offset)
{
    float64* valuePtr = (float64*)((char*)reflectionTarget + offset);
    float64 value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);
    
    // ImGui::Text(text.c_str());

    return *valuePtr;
}

bool UiReflector::consumeBool(FieldNameString name, uint32 offset)
{
    bool* valuePtr = (bool*)((char*)reflectionTarget + offset);
    bool value = *valuePtr;

    // std::string text = name + ": " + std::to_string(value);
    
    // ImGui::Text(text.c_str());

    return *valuePtr;
}

uint32 UiReflector::consumeEnum(FieldNameString name, uint32 offset, EnumValueNameString* enumNames, uint32 enumValueCount)
{
    // TODO: not sure how this should work...
    
    uint32* valuePtr = (uint32*)((char*)reflectionTarget + offset);
    uint32 value = *valuePtr;

    assert(value < enumValueCount);
    
    EnumValueNameString* selectedValueName = enumNames + value;

    if (ImGui::TreeNode((name + ": " + *selectedValueName).cstr()))
    {
        for (uint32 i = 0; i < enumValueCount; i++)
        {
            bool selected = (i == value);
            EnumValueNameString* valueName = enumNames + i;
            
            if (ImGui::Selectable(valueName->cstr(), selected))
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
#include "Game.h"
bool testUiReflection(Game* game, Entity e)
{
    // TODO: handle component groups
    UiReflector reflector;

    bool shouldStayOpen = true;
    if (!reflector.startReflection(e.friendlyName))
    {
        shouldStayOpen = false;
    }

    // Get components
    TransformComponent* transform = getTransformComponent(e);
    ColliderComponent* collider = getColliderComponent(e);
    CameraComponent* camera = getCameraComponent(e);
    PortalComponent* portal = getPortalComponent(e);
    DirectionalLightComponent* directionalLights = getDirectionalLightComponent(e);
    ComponentGroup<PointLightComponent> pointLights = getPointLightComponents(e);
    ComponentGroup<RenderComponent> renderComponents = getRenderComponents(e);

    //
    // Docked buttons
    //
    {
        bool sameLine = false;
        if (renderComponents.numComponents > 0 && transform != nullptr)
        {
            sameLine = true;
            ImGui::Checkbox("aabb", &game->drawAabb);
            
            if (game->drawAabb)
            {
                DebugDraw::instance().drawAabb(e);
            }
        }

        if (collider != nullptr)
        {
            if (sameLine) ImGui::SameLine();
            
            ImGui::Checkbox("coll", &game->drawCollider);
            if (game->drawCollider)
            {
                DebugDraw::instance().drawCollider(collider);
            }
        }
    }

    //
    // Component reflection
    //
    {
        if (transform)
        {
            if (ImGui::CollapsingHeader("Transform"))
            {
                reflector.reflectionTarget = transform;
                reflectTransformComponent(&reflector, 0);
            }
        }
    
        if (collider)
        {
            if (ImGui::CollapsingHeader("Collider"))
            {
                reflector.reflectionTarget = collider;
                reflectColliderComponent(&reflector, collider, 0);
            }
        }

        // TODO
    
        // if (camera)
        // {
        //     if (ImGui::CollapsingHeader("Camera"))
        //     {
        //         reflector.reflectionTarget = camera;
        //         reflectCameraComponent(&reflector, camera, 0);
        //     }
        // }
    
        // if (directionalLight)
        // {
        //     if (ImGui::CollapsingHeader("Directional Light"))
        //     {
        //         reflector.reflectionTarget = directionalLight;
        //         reflectDirectionalLightComponent(&reflector, 0);
        //     }
        // }
    
        // if (pointLight)
        // {
        //     if (ImGui::CollapsingHeader("Point Light"))
        //     {
        //         reflector.reflectionTarget = pointLight;
        //         reflectPointLightComponent(&reflector, 0);
        //     }
        // }
    
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
    }
    

    reflector.endReflection();
    return shouldStayOpen;
}

//
// Enums whose names are exposed via reflection
//
EnumValueNameString Axis3DNames[(uint32)Axis3D::ENUM_VALUE_COUNT] = { "X", "Y", "Z" };
EnumValueNameString ColliderTypeNames[(uint32)ColliderType::ENUM_VALUE_COUNT] = { "Rect3", "Sphere", "Cylinder", "Capsule" };
