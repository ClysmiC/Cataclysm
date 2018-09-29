#include "Transform.h"

ITransform::ITransform()
    : ITransform(Vec3(0, 0, 0), Quaternion(), Vec3(1, 1, 1))
{
}

ITransform::ITransform(Vec3 position)
    : ITransform(position, Quaternion(), Vec3(1, 1, 1))
{
}

ITransform::ITransform(Vec3 position, Quaternion orientation)
    : ITransform(position, orientation, Vec3(1, 1, 1))
{
}

ITransform::ITransform(Vec3 position, Quaternion orientation, Vec3 scale)
    : _localPosition(position), _localOrientation(orientation), _localScale(scale)
{
}

Vec3
ITransform::position()
{
    if (worldDirty) recalculateWorld();
    return worldPosition;
}

Quaternion
ITransform::orientation()
{
    if (worldDirty) recalculateWorld();
    return worldOrientation;
}

Vec3
ITransform::scale()
{
    if (worldDirty) recalculateWorld();
    return worldScale;
}

Vec3
ITransform::localPosition()
{
    return _localPosition;
}

Quaternion
ITransform::localOrientation()
{
    return _localOrientation;
}

Vec3
ITransform::localScale()
{
    return _localScale;
}

Vec3
ITransform::left()
{
    Vec3 result = this->orientation() * Vec3(-1, 0, 0);
    return result;
}

Vec3
ITransform::right()
{
    Vec3 result = this->orientation() * Vec3(1, 0, 0);
    return result;
}

Vec3
ITransform::up()
{
    Vec3 result = this->orientation() * Vec3(0, 1, 0);
    return result;
}

Vec3
ITransform::down()
{
    Vec3 result = this->orientation() * Vec3(0, -1, 0);
    return result;
}

Vec3
ITransform::forward()
{
    Vec3 result = this->orientation() * Vec3(0, 0, -1);
    return result;
}

Vec3
ITransform::back()
{
    Vec3 result = this->orientation() * Vec3(0, 0, 1);
    return result;
}

Mat4
ITransform::matrix()
{
    if (worldMatrixDirty)
    {
        recalculateWorld();
        
        this->toWorld.identityInPlace(); // reset to identity
        
        this->toWorld.scaleInPlace(this->scale());
        this->toWorld.rotateInPlace(this->orientation());
        this->toWorld.translateInPlace(this->position());
        worldMatrixDirty = false;
    }

    
    return this->toWorld;
}

void
ITransform::setLocalPosition(Vec3 position)
{
    _localPosition = position;
    markSelfAndChildrenDirty();
}

void
ITransform::setLocalOrientation(Quaternion orientation)
{
    _localOrientation = orientation;
    markSelfAndChildrenDirty();
}
void
ITransform::setLocalScale(Vec3 scale)
{
    _localScale = scale;
    markSelfAndChildrenDirty();
}
void
ITransform::setPosition(Vec3 position)
{
    ITransform* p = this->getParent();
    if (p)
    {
        Mat4 worldToParent = inverse(p->matrix());
        Vec4 newLocal = worldToParent * Vec4(position, 1.0);
        this->setLocalPosition(newLocal.xyz());
    }
    else
    {
        this->setLocalPosition(position);
    }
}

void
ITransform::setOrientation(Quaternion orientation)
{
    ITransform* p = this->getParent();
    if (p)
    {
        Quaternion rotationNeeded = relativeRotation(p->orientation(), orientation);
        this->setLocalOrientation(rotationNeeded);
    }
    else
    {
        this->setLocalOrientation(orientation);
    }
}

void
ITransform::setScale(Vec3 scale)
{
    ITransform* p = this->getParent();
    if (p)
    {
        this->setLocalScale(hadamardDivide(scale, p->scale()));
    }
    else
    {
        this->setLocalScale(scale);
    }
}

void
ITransform::setLocalPosition(float32 x, float32 y, float32 z)
{
    setLocalPosition(Vec3(x, y, z));
}

void
ITransform::setLocalOrientation(float32 x, float32 y, float32 z, float32 w)
{
    setLocalOrientation(Quaternion(x, y, z, w));
}

void
ITransform::setLocalScale(float32 x, float32 y, float32 z)
{
    setLocalScale(Vec3(x, y, z));
}

void
ITransform::setPosition(float32 x, float32 y, float32 z)
{
    setPosition(Vec3(x, y, z));
}

void
ITransform::setOrientation(float32 x, float32 y, float32 z, float32 w)
{
    setOrientation(Quaternion(x, y, z, w));
}

void
ITransform::setScale(float32 x, float32 y, float32 z)
{
    setScale(Vec3(x, y, z));
}

void
ITransform::markSelfAndChildrenDirty()
{
    worldDirty = true;
    worldMatrixDirty = true;
    
    auto children = this->getChildren();
    for (auto t : children)
    {
        t->markSelfAndChildrenDirty();
    }
}

void
ITransform::recalculateWorld()
{
    ITransform* p = this->getParent();

    if (p)
    {
        multiplyTransforms(
            this->_localPosition, this->_localOrientation, this->_localScale,
            p->position(), p->orientation(), p->scale(),
            &this->worldPosition, &this->worldOrientation, &this->worldScale
        );
    }
    else
    {
        this->worldPosition = this->_localPosition;
        this->worldOrientation = this->_localOrientation;
        this->worldScale = this->_localScale;
    }

    worldDirty = false;
}

Transform::Transform()
    : Transform(Vec3(0, 0, 0), Quaternion(), Vec3(1, 1, 1))
{
}

Transform::Transform(Vec3 position)
    : Transform(position, Quaternion(), Vec3(1, 1, 1))
{
}

Transform::Transform(Vec3 position, Quaternion orientation)
    : Transform(position, orientation, Vec3(1, 1, 1))
{
}

Transform::Transform(Vec3 position, Quaternion orientation, Vec3 scale)
    : ITransform(position, orientation, scale)
{
}

ITransform*
Transform::getParent()
{
    return nullptr;
}

std::vector<ITransform*>
Transform::getChildren()
{
    std::vector<ITransform*> result;
    return result;
}

void multiplyTransforms(
    Vec3 aToBPos, Quaternion aToBOrientation, Vec3 aToBScale,
    Vec3 bToCPos, Quaternion bToCOrientation, Vec3 bToCScale,
    Vec3* outPosition, Quaternion* outOrientation, Vec3* outScale
)
{
    *outOrientation = bToCOrientation * aToBOrientation;
    *outScale = hadamard(bToCScale, aToBScale);
    *outPosition = bToCOrientation * hadamard(bToCScale, aToBPos) + bToCPos;
}

Mat4 worldToView(ITransform* cameraXfm)
{
    Vec3 up = cameraXfm->up();
    Vec3 right = cameraXfm->right();
    Vec3 back = cameraXfm->back(); // NOTE: This is direction pointing TOWARDS the camera (i.e., -forward)

    Mat4 result;
    
    // Rotate
    result[0][0] = right.x;
    result[1][0] = up.x;
    result[2][0] = back.x;

    result[0][1] = right.y;
    result[1][1] = up.y;
    result[2][1] = back.y;
        
    result[0][2] = right.z;
    result[1][2] = up.z;
    result[2][2] = back.z;

    // Homogeneous
    result[3][3] = 1;
    
    // Translate
    Mat4 translation = translationMatrix(-cameraXfm->position());
    result = result * translation;

    return result;
}


