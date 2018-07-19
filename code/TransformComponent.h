#pragma once

#include "als_math.h"
#include "Component.h"
#include "Transform.h"

struct TransformComponent : public Component, public Transform
{
    Entity parent;
    std::vector<TransformComponent*> children;
private:
    Transform localTransform;

    bool cachedWorldDirty;
    Transform cachedWorldTransform;

public:
    TransformComponent();
    TransformComponent(Vec3 position);
    TransformComponent(Vec3 position, Quaternion orientation);
    TransformComponent(Vec3 position, Quaternion orientation, Vec3 scale);

    void setLocalPosition(Vec3 position);
    Vec3 localPosition();
    
    void setLocalOrientation(Quaternion orientation);
    Quaternion localOrientation();
    
    void setLocalScale(Vec3 scale);
    Vec3 localScale();
    
    void setWorldPosition(Vec3);
    Vec3 worldPosition();
    
    void setWorldOrientation(Quaternion);
    Quaternion worldOrientation();
    
    void setWorldScale(Vec3);
    Vec3 worldScale();
    
    Transform worldTransform();
    
    void recalculateWorld();
    
    void markSelfAndChildrenDirty();
};

