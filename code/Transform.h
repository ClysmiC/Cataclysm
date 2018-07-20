#pragma once

#include "als_math.h"
#include <vector>

struct Transform;
struct LiteTransform
{
    Vec3 position;
    Quaternion orientation;
    Vec3 scale;

    LiteTransform() = default;
    LiteTransform(Vec3 position, Quaternion orientation, Vec3 scale);
    LiteTransform(const Transform& transform);
}
    
struct Transform
{
    Transform();
    Transform(Vec3 position);
    Transform(Vec3 position, Quaternion orientation);
    Transform(Vec3 position, Quaternion orientation, Vec3 scale);
    Transform(const LiteTransform& transform);

    virtual Transform* getParent();
    virtual std::vector<Transform*> getChildren();
    
    inline Vec3 position()
    {
        if (dirty) recalculateWorld();
        return worldPosition;
    }

    inline Quaternion orientation()
    {
        if (dirty) recalculateWorld();
        return worldOrientation;
    }

    inline Vec3 scale()
    {
        if (dirty) recalculateWorld();
        return worldScale;
    }

    inline Vec3 localPosition()
    {
        return _localPosition;
    }

    inline Quaternion localOrientation()
    {
        return _localOrientation;
    }

    inline Vec3 localScale()
    {
        return _localScale;
    }

    inline Vec3 left()
    {
        Vec3 result = this->orientation() * Vec3(-1, 0, 0);
        return result;
    }

    inline Vec3 right()
    {
        Vec3 result = this->orientation() * Vec3(1, 0, 0);
        return result;
    }

    inline Vec3 up()
    {
        Vec3 result = this->orientation() * Vec3(0, 1, 0);
        return result;
    }

    inline Vec3 down()
    {
        Vec3 result = this->orientation() * Vec3(0, -1, 0);
        return result;
    }

    inline Vec3 forward()
    {
        Vec3 result = this->orientation() * Vec3(0, 0, -1);
        return result;
    }

    inline Vec3 back()
    {
        Vec3 result = this->orientation() * Vec3(0, 0, 1);
        return result;
    }

    Mat4 matrix();

    void setLocalPosition(Vec3 position);
    void setLocalOrientation(Quaternion orientation);
    void setLocalScale(Vec3 scale);
    void setPosition(Vec3 position);
    void setOrientation(Quaternion orientation);
    void setScale(Vec3 scale);

    void markSelfAndChildrenDirty();
    void recalculateWorld();
    
protected:
    //
    // Authoratative
    //
    Vec3 _localPosition;
    Quaternion _localOrientation;
    Vec3 _localScale;

    //
    // Cached
    //
    bool dirty = true;
    Vec3 worldPosition;
    Quaternion worldOrientation;
    Vec3 worldScale;

    Mat4 toParent;
    Mat4 toWorld;
};



//
// Returns the transform aToC in the out parameters
//
void multiplyTransforms(
    Vec3 aToBPos, Quaternion aToBOrientation, Vec3 aToBScale,
    Vec3 bToCPos, Quaternion bToCOrientation, Vec3 bToCScale,
    Vec3* outPosition, Quaternion* outOrientation, Vec3* outScale
);

// TODO: put this in camera component code?
Mat4 worldToView(Transform* cameraXfm);
