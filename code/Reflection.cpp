#include "Reflection.h"
#include "imgui/imgui.h"

#include "ecs/components/TransformComponent.h"
#include "ecs/components/ConvexHullColliderComponent.h"
#include "ecs/components/ColliderComponent.h"
#include "ecs/components/CameraComponent.h"
#include "ecs/components/DirectionalLightComponent.h"
#include "ecs/components/EntityDetails.h"
#include "ecs/components/PointLightComponent.h"
#include "ecs/components/PortalComponent.h"
#include "ecs/components/RenderComponent.h"

#include "resource/resources/Mesh.h"

#include "Editor.h"

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

void reflectVec3Euler(IReflector* reflector, uint32 startingOffset)
{
    reflector->consumeFloat32Euler("X", startingOffset + offsetof(Vec3, x));
    reflector->consumeFloat32Euler("Y", startingOffset + offsetof(Vec3, y));
    reflector->consumeFloat32Euler("Z", startingOffset + offsetof(Vec3, z));
}

void reflectVec3Normalized(IReflector* reflector, uint32 startingOffset)
{
    Vec3* v = (Vec3*)((char*)reflector->reflectionTarget() + startingOffset);

    reflector->pushReflectionTarget(v);
    {
        reflector->consumeFloat32NormalizedVec3("X", offsetof(Vec3, x));
        reflector->consumeFloat32NormalizedVec3("Y", offsetof(Vec3, y));
        reflector->consumeFloat32NormalizedVec3("Z", offsetof(Vec3, z));
    }

    v->normalizeInPlace();
    reflector->popReflectionTarget();
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
    if (reflector->useEulerAngles)
    {
        Quaternion* q = (Quaternion*)((char*)reflector->reflectionTarget() + startingOffset);
        Vec3 eulerBeforeReflect = toEuler(*q);
        Vec3 euler = eulerBeforeReflect;

        reflector->pushReflectionTarget(&euler);
        {
            reflectVec3Euler(reflector, 0);

            if (!equals(eulerBeforeReflect, euler)) // && reflector->recanonicalizeEuler)
            {
                *q = fromEuler(euler);
            }
        }
        reflector->popReflectionTarget();
    }
    else
    {
        reflector->consumeFloat32("X", startingOffset + offsetof(Quaternion, x));
        reflector->consumeFloat32("Y", startingOffset + offsetof(Quaternion, y));
        reflector->consumeFloat32("Z", startingOffset + offsetof(Quaternion, z));
        reflector->consumeFloat32("W", startingOffset + offsetof(Quaternion, w));
    }
}

void reflectEntityDetailsComponent (IReflector* reflector, uint32 startingOffset)
{
    if (reflector->purpose != ReflectionPurpose::UI)
    {
        // TODO: reflect other stuff (e.g. for serialization in the future)
    }

    reflector->consumeString16("Name", startingOffset + offsetof(EntityDetails, friendlyName));
}

void reflectTransformComponent(IReflector* reflector, uint32 startingOffset)
{
    if (reflector->useLocalXfm)
    {
        TransformComponent* xfm = (TransformComponent*)((char*)reflector->reflectionTarget() + startingOffset);
        
        if (reflector->pushStruct("Position"))
        {
            Vec3 localPositionBeforeReflect = xfm->localPosition();
            Vec3 localPosition = localPositionBeforeReflect;
            
            reflector->pushReflectionTarget(&localPosition);
            {
                reflectVec3(reflector, 0);

                if (!equals(localPosition, localPositionBeforeReflect))
                {
                    xfm->setLocalPosition(localPosition);
                }
            }
            reflector->popReflectionTarget();
            
            reflector->popStruct();
        }

        if (reflector->pushStruct("Orientation"))
        {
            Quaternion localOrientationBeforeReflect = xfm->localOrientation();
            Quaternion localOrientation = localOrientationBeforeReflect;
            
            reflector->pushReflectionTarget(&localOrientation);
            {
                reflectQuaternion(reflector, 0);

                if (!equals(localOrientation, localOrientationBeforeReflect))
                {
                    xfm->setLocalOrientation(localOrientation);
                }
            }
            reflector->popReflectionTarget();
            
            reflector->popStruct();
        }

        if (reflector->pushStruct("Scale"))
        {
            Vec3 localScaleBeforeReflect = xfm->localScale();
            Vec3 localScale = localScaleBeforeReflect;
            
            reflector->pushReflectionTarget(&localScale);
            {
                reflectVec3(reflector, 0);

                if (!equals(localScale, localScaleBeforeReflect))
                {
                    xfm->setLocalScale(localScale);
                }
            }
            reflector->popReflectionTarget();
            
            reflector->popStruct();
        }
    }
    else
    {
        // TODO: support world transform
    }
}

void reflectConvexHullColliderComponent(IReflector * reflector, uint32 startingOffset)
{
    ConvexHullColliderComponent* chcc = (ConvexHullColliderComponent*)((char*)reflector->reflectionTarget() + startingOffset);
    reflector->consumeBool("Show in editor?", startingOffset + offsetof(ConvexHullColliderComponent, showInEditor));
}

void reflectColliderComponent(IReflector* reflector, uint32 startingOffset)
{
    ColliderComponent* collider = (ColliderComponent*)((char*)reflector->reflectionTarget() + startingOffset);
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

void reflectCameraComponent(IReflector* reflector, uint32 startingOffset)
{
    CameraComponent* camera = (CameraComponent*)((char*)reflector->reflectionTarget() + startingOffset);

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
        reflectVec3Normalized(reflector, startingOffset + offsetof(DirectionalLightComponent, direction));
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
    reflector->consumeUInt32(
        "Linked ID",
        startingOffset + offsetof(PortalComponent, connectedPortal) + offsetof(PotentiallyStaleEntity, id),
        ReflectionFlag_DontAutoUpdate
    );
}

void reflectRenderComponent(IReflector* reflector, uint32 startingOffset)
{
    RenderComponent* rc = (RenderComponent*)((char*)reflector->reflectionTarget() + startingOffset);
    reflector->consumeBool("Visible?", startingOffset + offsetof(RenderComponent, isVisible));

    Mesh* mesh = rc->submesh->mesh;

    reflector->pushReflectionTarget(mesh);
    reflector->consumeString256("Mesh ID", startingOffset + offsetof(Mesh, id), ReflectionFlag_ReadOnly);
    reflector->popReflectionTarget();
}

IReflector::IReflector(ReflectionPurpose purpose)
    : purpose(purpose)
{
}

UiReflector::UiReflector()
    : IReflector(ReflectionPurpose::UI)
{
    this->useEulerAngles = true;
}

bool UiReflector::startReflection(EntityNameString label)
{
    return true; // @todo: revisit if we need this api
}

void UiReflector::endReflection()
{
}

bool UiReflector::pushStruct(StructNameString name)
{
    bool result = ImGui::TreeNode(name.cstr());
    
    return result;
}

void UiReflector::popStruct()
{
    ImGui::TreePop();
}

int32 UiReflector::consumeInt32(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    int32* valuePtr = (int32*)((char*)reflectionTarget() + offset);
    int32 valueCopy = *valuePtr;

    ImGuiInputTextFlags imguiFlags = 0;
    if (flags & ReflectionFlag_ReadOnly) imguiFlags       |= ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly;
    if (flags & ReflectionFlag_DontAutoUpdate) imguiFlags |= ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputScalar(name.cstr(), ImGuiDataType_S32, &valueCopy, nullptr, nullptr, nullptr, imguiFlags))
    {
        // This updates the actual value
        *valuePtr = valueCopy;
    }

    return *valuePtr;
}

uint32 UiReflector::consumeUInt32(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    uint32* valuePtr = (uint32*)((char*)reflectionTarget() + offset);
    uint32 valueCopy = *valuePtr;

    ImGuiInputTextFlags imguiFlags = 0;
    if (flags & ReflectionFlag_ReadOnly) imguiFlags       |= ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly;
    if (flags & ReflectionFlag_DontAutoUpdate) imguiFlags |= ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputScalar(name.cstr(), ImGuiDataType_U32, &valueCopy, nullptr, nullptr, nullptr, imguiFlags))
    {
        // This updates the actual value
        *valuePtr = valueCopy;
    }

    return *valuePtr;
}

float32 UiReflector::consumeFloat32(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    float32* valuePtr = (float32*)((char*)reflectionTarget() + offset);
    float32 valueCopy = *valuePtr;

    ImGuiInputTextFlags imguiFlags = 0;
    if (flags & ReflectionFlag_ReadOnly) imguiFlags |= ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly;
        
    if (ImGui::InputScalar(name.cstr(), ImGuiDataType_Float, &valueCopy))
    {
        // This updates the actual value
        *valuePtr = valueCopy;
    }

    return *valuePtr;
}

float64 UiReflector::consumeFloat64(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    float64* valuePtr = (float64*)((char*)reflectionTarget() + offset);
    float64 valueCopy = *valuePtr;

    ImGuiInputTextFlags imguiFlags = 0;
    if (flags & ReflectionFlag_ReadOnly) imguiFlags |= ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly;
        
    if (ImGui::InputScalar(name.cstr(), ImGuiDataType_Double, &valueCopy))
    {
        // This updates the actual value
        *valuePtr = valueCopy;
    }

    return *valuePtr;
}

bool UiReflector::consumeBool(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    bool* valuePtr = (bool*)((char*)reflectionTarget() + offset);
    bool valueCopy = *valuePtr;

    if (ImGui::Checkbox(name.cstr(), &valueCopy))
    {
        *valuePtr = valueCopy;
    }

    return *valuePtr;
}

uint32 UiReflector::consumeEnum(FieldNameString name, uint32 offset, EnumValueNameString* enumNames, uint32 enumValueCount, ReflectionFlags flags)
{    
    uint32* valuePtr = (uint32*)((char*)reflectionTarget() + offset);
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

float32 UiReflector::consumeFloat32Euler(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    float32* valuePtr = (float32*)((char*)this->reflectionTarget() + offset);
    uint32 parentId = ImGui::GetID("");
    bool isHot = this->editor->componentList.lastFrameActiveId == parentId;

    if (isHot)
    {
        float32 nonCanonicalValue;
        if (name == "x" || name == "X")
        {
            nonCanonicalValue = this->editor->componentList.hotXfmEuler.x;
        }
        else if (name == "y" || name == "Y")
        {
            nonCanonicalValue = this->editor->componentList.hotXfmEuler.y;
        }
        else if (name == "z" || name == "Z")
        {
            nonCanonicalValue = this->editor->componentList.hotXfmEuler.z;
        }
        else
        {
            assert(false);
        }
        
        this->pushReflectionTarget(&nonCanonicalValue);
        *valuePtr  = this->consumeFloat32(name, 0);
        this->popReflectionTarget();
    }
    else
    {
        *valuePtr = this->consumeFloat32(name, offset);
    }

    if (ImGui::IsItemActive())
    {
        this->editor->componentList.thisFrameActiveId = parentId;
    }

    if (isHot || this->editor->componentList.lastFrameActiveId == 0)
    {
        if (name == "x" || name == "X")
        {
            this->editor->componentList.hotXfmEuler.x = *valuePtr;
        }
        else if (name == "y" || name == "Y")
        {
            this->editor->componentList.hotXfmEuler.y = *valuePtr;
        }
        else if (name == "z" || name == "Z")
        {
            this->editor->componentList.hotXfmEuler.z = *valuePtr;
        }
        else
        {
            assert(false);
        }
    }

    return *valuePtr;
}

float32 UiReflector::consumeFloat32NormalizedVec3(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    float32* valuePtr = (float32*)((char*)this->reflectionTarget() + offset);
    uint32 parentId = ImGui::GetID("");
    bool isHot = this->editor->componentList.lastFrameActiveId == parentId;

    if (isHot)
    {
        float32 nonCanonicalValue;
        if (name == "x" || name == "X")
        {
            nonCanonicalValue = this->editor->componentList.hotNormalizedVec3.x;
        }
        else if (name == "y" || name == "Y")
        {
            nonCanonicalValue = this->editor->componentList.hotNormalizedVec3.y;
        }
        else if (name == "z" || name == "Z")
        {
            nonCanonicalValue = this->editor->componentList.hotNormalizedVec3.z;
        }
        else
        {
            assert(false);
        }
        
        this->pushReflectionTarget(&nonCanonicalValue);
        *valuePtr  = this->consumeFloat32(name, 0);
        this->popReflectionTarget();
    }
    else
    {
        *valuePtr = this->consumeFloat32(name, offset);
    }

    if (ImGui::IsItemActive())
    {
        this->editor->componentList.thisFrameActiveId = parentId;
    }

    if (isHot || this->editor->componentList.lastFrameActiveId == 0)
    {
        if (name == "x" || name == "X")
        {
            this->editor->componentList.hotNormalizedVec3.x = *valuePtr;
        }
        else if (name == "y" || name == "Y")
        {
            this->editor->componentList.hotNormalizedVec3.y = *valuePtr;
        }
        else if (name == "z" || name == "Z")
        {
            this->editor->componentList.hotNormalizedVec3.z = *valuePtr;
        }
        else
        {
            assert(false);
        }
    }

    return *valuePtr;
}

string16 UiReflector::consumeString16(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    string16* valuePtr = (string16*)((char*)reflectionTarget() + offset);
    string16 value = *valuePtr;
    string16 valueCopy = value;

    ImGuiInputTextFlags imguiFlags = 0;
    if (flags & ReflectionFlag_ReadOnly) imguiFlags |= ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly;

    if (ImGui::InputText(name.cstr(), valueCopy.data, 16, imguiFlags))
    {
        // This updates the actual value
        *valuePtr = valueCopy;
        valuePtr->invalidateLength();
    }

    return *valuePtr;
}

string256 UiReflector::consumeString256(FieldNameString name, uint32 offset, ReflectionFlags flags)
{
    string256* valuePtr = (string256*)((char*)reflectionTarget() + offset);
    string256 value = *valuePtr;
    string256 valueCopy = value;

    ImGuiInputTextFlags imguiFlags = 0;
    if (flags & ReflectionFlag_ReadOnly) imguiFlags |= ImGuiInputTextFlags_::ImGuiInputTextFlags_ReadOnly;

    if (ImGui::InputText(name.cstr(), valueCopy.data, 16, imguiFlags))
    {
        // This updates the actual value
        *valuePtr = valueCopy;
        valuePtr->invalidateLength();
    }

    return *valuePtr;
}

//
// Enums whose names are exposed via reflection
//
EnumValueNameString Axis3DNames[(uint32)Axis3D::ENUM_VALUE_COUNT] = { "X", "Y", "Z" };
EnumValueNameString ColliderTypeNames[(uint32)ColliderType::ENUM_VALUE_COUNT] = { "Rect3", "Sphere", "Cylinder", "Capsule" };
