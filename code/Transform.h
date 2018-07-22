#pragma once

#include "als_math.h"
#include <vector>

struct ITransform
{
    ITransform();
    ITransform(Vec3 position);
    ITransform(Vec3 position, Quaternion orientation);
    ITransform(Vec3 position, Quaternion orientation, Vec3 scale);

    //
    // Details of how to retrieve these are left up to the
    // implementations
    //
    virtual ITransform* getParent() = 0;
    virtual std::vector<ITransform*> getChildren() = 0;
    
    Vec3 position();
    Quaternion orientation();
    Vec3 scale();
    
    Vec3 localPosition();
    Quaternion localOrientation();
    Vec3 localScale();

    Vec3 left();
    Vec3 right();
    Vec3 up();
    Vec3 down();
    Vec3 forward();
    Vec3 back();
    
    Mat4 matrix();

    void setLocalPosition(Vec3 position);
    void setLocalOrientation(Quaternion orientation);
    void setLocalScale(Vec3 scale);
    void setPosition(Vec3 position);
    void setOrientation(Quaternion orientation);
    void setScale(Vec3 scale);

    void recalculateWorld();
    
    void markSelfAndChildrenDirty();
    
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
    bool worldDirty = true;
    Vec3 worldPosition;
    Quaternion worldOrientation;
    Vec3 worldScale;

    bool worldMatrixDirty = true;
    Mat4 toWorld;
};

struct Transform : public ITransform
{
    Transform();
    Transform(Vec3 position);
    Transform(Vec3 position, Quaternion orientation);
    Transform(Vec3 position, Quaternion orientation, Vec3 scale);

    ITransform* getParent() override;
    std::vector<ITransform*> getChildren() override;
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
Mat4 worldToView(ITransform* cameraXfm);
