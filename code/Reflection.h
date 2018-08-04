#pragma once

#include "als_math.h"
#include "als_fixed_string.h"

#include "ColliderComponent.h" // needed for ColliderType enum
#include <stack>

struct CameraComponent;
struct EditorState;
struct EntityDetails;

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

typedef uint64 ReflectionFlags;
const static ReflectionFlags ReflectionFlag_ReadOnly = 1 << 0;

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
    virtual int32   consumeInt32  (FieldNameString name, uint32 offset, ReflectionFlags flags=0) = 0;
    virtual uint32  consumeUInt32 (FieldNameString name, uint32 offset, ReflectionFlags flags=0) = 0;
    virtual float32 consumeFloat32(FieldNameString name, uint32 offset, ReflectionFlags flags=0) = 0;
    virtual float64 consumeFloat64(FieldNameString name, uint32 offset, ReflectionFlags flags=0) = 0;
    virtual bool    consumeBool   (FieldNameString name, uint32 offset, ReflectionFlags flags=0) = 0;

    // TODO: templatize this?
    virtual string16 consumeString16(FieldNameString name, uint32 offset, ReflectionFlags flags=0) = 0;

    //
    // This may need special treatment (i.e., UI uses the stored euler value instead of the canonical one)
    // until the user ends their input
    //
    virtual float32 consumeFloat32Euler(FieldNameString name, uint32 offset, ReflectionFlags flags=0) { return consumeFloat32(name, offset, flags); };

    // Note: Consumable enums must have underlying type uint32
    // Potential todo: make consumeEnumInt32, consumeEnumUInt32, etc. (but I think it is fine to assume uint32 and enforce
    // that for now)
    virtual uint32  consumeEnum(FieldNameString name, uint32 offset, EnumValueNameString* enumNames, uint32 enumValueCount, ReflectionFlags flags=0) = 0;
    
private:
    std::stack<void*> reflectionTargets;
};

struct UiReflector : public IReflector
{
    UiReflector();

    EditorState* editor;
    
    bool pushStruct(StructNameString name) override;
    void popStruct() override;

    bool startReflection(EntityNameString label) override;
    void endReflection() override;
    
    int32   consumeInt32  (FieldNameString name, uint32 offset, ReflectionFlags flags=0) override;
    uint32  consumeUInt32 (FieldNameString name, uint32 offset, ReflectionFlags flags=0) override;
    float32 consumeFloat32(FieldNameString name, uint32 offset, ReflectionFlags flags=0) override;
    float64 consumeFloat64(FieldNameString name, uint32 offset, ReflectionFlags flags=0) override;
    bool    consumeBool   (FieldNameString name, uint32 offset, ReflectionFlags flags=0) override;
    
    string16 consumeString16(FieldNameString name, uint32 offset, ReflectionFlags flags=0) override;
        
    uint32  consumeEnum   (FieldNameString name, uint32 offset, EnumValueNameString* enumNames, uint32 enumValueCount, ReflectionFlags flags=0) override;

    float32 consumeFloat32Euler(FieldNameString name, uint32 offset, ReflectionFlags flags=0) override;
};

struct Game;
bool testUiReflection(Game* game, Entity e);

void reflectVec2(IReflector* reflector, uint32 startingOffset);
void reflectVec3(IReflector* reflector, uint32 startingOffset);
void reflectVec3Rgb(IReflector* reflector, uint32 startingOffset);
void reflectVec3Euler(IReflector* reflector, uint32 startingOffset);
void reflectVec4(IReflector* reflector, uint32 startingOffset);
void reflectQuaternion(IReflector* reflector, uint32 startingOffset);

void reflectEntityDetailsComponent   (IReflector* reflector, EntityDetails* details, uint32 startingOffset);
void reflectTransformComponent       (IReflector* reflector, uint32 startingOffset);
void reflectColliderComponent        (IReflector* reflector, ColliderComponent* collider, uint32 startingOffset);
void reflectCameraComponent          (IReflector* reflector, CameraComponent* camera, uint32 startingOffset);
void reflectDirectionalLightComponent(IReflector* reflector, uint32 startingOffset);
void reflectPointLightComponent      (IReflector* reflector, uint32 startingOffset);
void reflectPortalComponent          (IReflector* reflector, uint32 startingOffset);
void reflectRenderComponent          (IReflector* reflector, uint32 startingOffset);

//
// Enums whose names are exposed via reflection
//
extern EnumValueNameString Axis3DNames[(uint32)Axis3D::ENUM_VALUE_COUNT];
extern EnumValueNameString ColliderTypeNames[(uint32)ColliderType::ENUM_VALUE_COUNT];
