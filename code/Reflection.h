#pragma once

#include "als_math.h"
#include "als_fixed_string.h"

#include "ColliderComponent.h" // needed for ColliderType enum
#include <stack>

struct CameraComponent;

enum class ReflectionPurpose
{
    OTHER,
    UI,
    SERIALIZATION
};

typedef string16 EnumValueNameString;
typedef string16 StructNameString;
typedef string16 FieldNameString;
typedef string16 EntityNameString;

struct IReflector
{
    IReflector() = default;
    IReflector(ReflectionPurpose purpose);
    
    ReflectionPurpose purpose = ReflectionPurpose::OTHER;
    bool useLocalXfm;
    bool useEulerAngles;

    virtual void* reflectionTarget()
    {
        return reflectionTargets.top();
    }

    //
    // Sometimes you have to reflect "pseudo"-properties, such as the local xfm
    // in a TransformComponent (despite it being stored in memory world space).
    //
    // To reflect into any auxiliary data such as this, you can push a reflection
    // target (and then pop it when done) to not lose context of the primary thing
    // that you are reflecting.
    //
    virtual void setPrimaryReflectionTarget(void* target)
    {
        while(!reflectionTargets.empty()) reflectionTargets.pop();
        reflectionTargets.push(target);
    }
    
    virtual void pushReflectionTarget(void* target)
    {
        reflectionTargets.push(target);
    }

    virtual void popReflectionTarget()
    {
        return reflectionTargets.pop();
    }
    
    virtual bool pushStruct(StructNameString name) = 0;
    virtual void popStruct() = 0;
    
    virtual bool startReflection(EntityNameString label) = 0;
    virtual void endReflection() = 0;

    //
    // "Consume" is "end of the line" for reflection. It operates on primitives.
    //
    virtual int32   consumeInt32(FieldNameString name, uint32 offset) = 0;
    virtual uint32  consumeUInt32(FieldNameString name, uint32 offset) = 0;
    virtual float32 consumeFloat32(FieldNameString name, uint32 offset) = 0;
    virtual float64 consumeFloat64(FieldNameString name, uint32 offset) = 0;
    virtual bool    consumeBool(FieldNameString name, uint32 offset) = 0;

    // Note: Consumable enums must have underlying type uint32
    // Potential todo: make consumeEnumInt32, consumeEnumUInt32, etc. (but I think it is fine to assume uint32 and enforce
    // that for now)
    virtual uint32  consumeEnum(FieldNameString name, uint32 offset, EnumValueNameString* enumNames, uint32 enumValueCount) = 0;
    
private:
    std::stack<void*> reflectionTargets;
};

struct UiReflector : public IReflector
{
    UiReflector();
    
    uint32 indentationLevel = 0;
    
    bool pushStruct(StructNameString name) override;
    void popStruct() override;

    bool startReflection(EntityNameString label) override;
    void endReflection() override;
    
    int32   consumeInt32(FieldNameString name, uint32 offset) override;
    uint32  consumeUInt32(FieldNameString name, uint32 offset) override;
    float32 consumeFloat32(FieldNameString name, uint32 offset) override;
    float64 consumeFloat64(FieldNameString name, uint32 offset) override;
    bool    consumeBool(FieldNameString name, uint32 offset) override;
    uint32  consumeEnum(FieldNameString name, uint32 offset, EnumValueNameString* enumNames, uint32 enumValueCount) override;
};

struct Game;
bool testUiReflection(Game* game, Entity e);

void reflectVec2(IReflector* reflector, uint32 startingOffset);
void reflectVec3(IReflector* reflector, uint32 startingOffset);
void reflectVec4(IReflector* reflector, uint32 startingOffset);
void reflectQuaternion(IReflector* reflector, uint32 startingOffset);

void reflectTransformComponent(IReflector* reflector, uint32 startingOffset);
void reflectColliderComponent(IReflector* reflector, ColliderComponent* collider, uint32 startingOffset);
void reflectCameraComponent(IReflector* reflector, CameraComponent* camera, uint32 startingOffset);
void reflectDirectionalLightComponent(IReflector* reflector, uint32 startingOffset);
void reflectPointLightComponent(IReflector* reflector, uint32 startingOffset);
void reflectPortalComponent(IReflector* reflector, uint32 startingOffset);
void reflectRenderComponent(IReflector* reflector, uint32 startingOffset);

//
// Enums whose names are exposed via reflection
//
extern EnumValueNameString Axis3DNames[(uint32)Axis3D::ENUM_VALUE_COUNT];
extern EnumValueNameString ColliderTypeNames[(uint32)ColliderType::ENUM_VALUE_COUNT];
