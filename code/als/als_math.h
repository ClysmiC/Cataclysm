#pragma once

#include "math.h"
#include "assert.h"
#include "als_types.h"

#define FLOAT_EQ(x, y, epsilon) (fabs((x) - (y)) < epsilon)
#define PI 3.14159265359f
#define EPSILON 0.0001f
#define TO_RAD(x) ((x)* PI / 180.0f)
#define TO_DEG(x) ((x)* 180.0f/ PI)

constexpr int64 powi(int64 base, uint64 power)
{
    int64 result = 1;
    for(uint64 i = 0; i < power; i++)
    {
        result *= base;
    }

    return result;
}

enum class Axis3D : uint32
{
    X,
    Y,
    Z,

    ENUM_VALUE_COUNT
};

union Vec2
{
    Vec2();
    explicit Vec2(float32 value);
    Vec2(float32 x, float32 y);
    
    struct
    {
        float32 x, y;
    };
    float32 element[2];

    Vec2& normalizeOrZeroInPlace();
    Vec2& normalizeOrXAxisInPlace();
    Vec2& normalizeInPlace();
};

union Vec3
{
    Vec3();
    explicit Vec3(Axis3D unitAxis);
    explicit Vec3(float32 value);
    Vec3(Vec2 xy, float32 z);
    Vec3(float32 x, float32 y, float32 z);
    
    struct
    {
        float32 x, y, z;
    };
    struct
    {
        float32 r, g, b;
    };
    float32 element[3];

    Vec3& normalizeOrZeroInPlace();
    Vec3& normalizeOrXAxisInPlace();
    Vec3& normalizeInPlace();

    inline Vec2 xy() { return Vec2(x, y); }
    inline Vec2 xz() { return Vec2(x, z); }
    inline Vec2 yz() { return Vec2(y, z); }

    bool operator== (Vec3 other);
};

union Vec4
{
    Vec4();
    explicit Vec4(float32 value);
    Vec4(Vec2 xy, float32 z, float32 w);
    Vec4(Vec2 xy, Vec2 zw);
    Vec4(Vec3 xyz, float32 w);
    Vec4(float32 x, float32 y, float32 z, float32 w);
    
    struct
    {
        float32 x, y, z, w;
    };
    struct
    {
        float32 r, g, b, a;
    };
    float32 element[4];

    
    Vec4& normalizeOrZeroInPlace();
    Vec4& normalizeOrXAxisInPlace();
    Vec4& normalizeInPlace();

    inline Vec3 xyz() { return Vec3(x, y, z); }
};

union Quaternion
{
    Quaternion();
    Quaternion(Vec3 vector, float32 scalar);
    explicit Quaternion(Vec4 xyzw);
    Quaternion(float32 x, float32 y, float32 z, float32 w);
    
    struct
    {
        float32 x, y, z, w;
    };
    float32 element[4];

    Quaternion& normalizeInPlace();
    Quaternion& invertInPlace();
};

struct Mat4;
struct Mat3
{
    Mat3() = default;
    Mat3(Mat4 mat4);
    
    float32 _e[3][3] = {
        { 1, 0, 0},
        { 0, 1, 0},
        { 0, 0, 1}
    };

    // Only overloads the first index to point to the correct array, then
    // the second index is handled by built-in c++
    float32* operator [] (int32 i);
    float32* dataPointer();

    Mat3& identityInPlace();
    Mat3& transposeInPlace();
};

struct Mat4
{
    Mat4() = default;
    Mat4(Mat3 mat3);
    
    float32 _e[4][4] = {
        { 1, 0, 0, 0},
        { 0, 1, 0, 0},
        { 0, 0, 1, 0},
        { 0, 0, 0, 1}
    };

    // Only overloads the first index to point to the correct array, then
    // the second index is handled by built-in c++
    float32* operator [] (int32 i);
    float32* dataPointer();

    Mat4& scaleInPlace(Vec3 scale);
    Mat4& rotateInPlace(Quaternion rotation);
    Mat4& translateInPlace(Vec3 translation);

    Mat4& identityInPlace();
    Mat4& transposeInPlace();
    Mat4& mat3ifyInPlace();
    Mat4& perspectiveInPlace(float32 fov, float32 aspectRatio, float32 near, float32 far);
    Mat4& orthoInPlace(float32 width, float32 aspectRatio, float32 near, float32 far);
};

union Triangle
{
    struct
    {
        Vec3 a;
        Vec3 b;
        Vec3 c;
    };

    Vec3 elements[3];

    Triangle();
    Triangle(Vec3 a, Vec3 b, Vec3 c);
};

struct Plane
{
    Plane(Vec3 point, Vec3 normal);
    Plane(Vec3 pointA, Vec3 pointB, Vec3 pointC); // NOTE: normal calculated using CCW winding
    Vec3 point;
    Vec3 normal;
};

Vec2 operator + (Vec2 vectorA, Vec2 vectorB);
Vec2 operator + (Vec2 vector, float32 constant);
Vec2& operator += (Vec2& vectorA, Vec2 vectorB);
Vec2& operator += (Vec2& vector, float32 constant);

Vec2 operator - (Vec2 vectorA, Vec2 vectorB);
Vec2 operator - (Vec2 vector, float32 constant);
Vec2& operator -= (Vec2& vectorA, Vec2 vectorB);
Vec2& operator -= (Vec2& vector, float32 constant);
Vec2 operator - (Vec2 vector);


Vec2 operator * (float32 scalar, Vec2 vector);
Vec2 operator * (Vec2 vector, float32 scalar);
Vec2& operator *= (Vec2& vector, float32 scalar);

Vec2 operator / (Vec2 vector, float32 scalar);
Vec2& operator /= (Vec2& vector, float32 scalar);


// Vec3
Vec3 operator + (Vec3 vectorA, Vec3 vectorB);
Vec3 operator + (Vec3 vector, float32 constant);
Vec3& operator += (Vec3& vectorA, Vec3 vectorB);
Vec3& operator += (Vec3& vector, float32 constant);

Vec3 operator - (Vec3 vectorA, Vec3 vectorB);
Vec3 operator - (Vec3 vector, float32 constant);
Vec3& operator -= (Vec3& vectorA, Vec3 vectorB);
Vec3& operator -= (Vec3& vector, float32 constant);
Vec3 operator - (Vec3 vector);

Vec3 operator * (float32 scalar, Vec3 vector);
Vec3 operator * (Vec3 vector, float32 scalar);
Vec3& operator *= (Vec3& vector, float32 scalar);

Vec3 operator / (Vec3 vector, float32 scalar);
Vec3& operator /= (Vec3& vector, float32 scalar);

Vec3 operator * (Quaternion quaternion, Vec3 vector);
Vec3 operator * (Mat3 m, Vec3 v);

// Vec4
Vec4 operator + (Vec4 vectorA, Vec4 vectorB);
Vec4 operator + (Vec4 vector, float32 constant);
Vec4& operator += (Vec4& vectorA, Vec4 vectorB);
Vec4& operator += (Vec4& vector, float32 constant);

Vec4 operator - (Vec4 vectorA, Vec4 vectorB);
Vec4 operator - (Vec4 vector, float32 constant);
Vec4& operator -= (Vec4& vectorA, Vec4 vectorB);
Vec4& operator -= (Vec4& vector, float32 constant);
Vec4 operator - (Vec4 vector);

Vec4 operator * (float32 scalar, Vec4 vector);
Vec4 operator * (Vec4 vector, float32 scalar);
Vec4& operator *= (Vec4& vector, float32 scalar);
Vec4 operator / (Vec4 vector, float32 scalar);
Vec4& operator /= (Vec4& vector, float32 scalar);

Vec4 operator * (Mat4 m, Vec4 v);

// Quaternion
Quaternion operator + (Quaternion quatA, Quaternion quatB);
Quaternion operator + (Quaternion quat, float32 constant);
Quaternion& operator += (Quaternion& quatA, Quaternion quatB);
Quaternion& operator += (Quaternion& quat, float32 constant);

Quaternion operator - (Quaternion quatA, Quaternion quatB);
Quaternion operator - (Quaternion quat, float32 constant);
Quaternion& operator -= (Quaternion& quatA, Quaternion quatB);
Quaternion& operator -= (Quaternion& quat, float32 constant);
Quaternion operator - (Quaternion quat);

Quaternion operator * (float32 scalar, Quaternion quat);
Quaternion operator * (Quaternion quat, float32 scalar);
Quaternion& operator *= (Quaternion& quat, float32 scalar);
Quaternion operator / (Quaternion quat, float32 scalar);
Quaternion& operator /= (Quaternion& quat, float32 scalar);

Quaternion operator * (Quaternion a, Quaternion b);

// Mat
Mat3 operator * (Mat3 left, Mat3 right);
Mat3 operator * (Mat3 left, float32 right);
Mat3 operator * (float32 left, Mat3 right);

Mat4 operator * (Mat4 left, Mat4 right);
Mat4 operator * (Mat4 left, float32 right);
Mat4 operator * (float32 left, Mat4 right);

float32 determinant(Mat3 m);
Mat3 inverse(Mat3 m);

float32 determinant(Mat4 m);
Mat4 inverse(Mat4 m);

// Vec2
inline float32 dot(Vec2 vectorA, Vec2 vectorB)
{
    float32 result = vectorA.x * vectorB.x + vectorA.y * vectorB.y;
    return result;
}
inline Vec2 hadamard(Vec2 v1, Vec2 v2)
{
    Vec2 result = Vec2(v1.x * v2.x, v1.y * v2.y);
    return result;
}
inline Vec2 hadamardDivide(Vec2 v1, Vec2 v2)
{
    Vec2 result = Vec2(v1.x / v2.x, v1.y / v2.y);
    return result;
}
inline float32 lengthSquared(Vec2 v)
{
    float32 result = v.x * v.x + v.y * v.y;
    return result;
}
inline float32 length(Vec2 v)
{
    float32 result = sqrtf(lengthSquared(v));
    return result;
}
inline Vec2 normalize(Vec2 v)
{
    Vec2 result = v / length(v);
    return result;
}
inline Vec2 normalizeOrZero(Vec2 v)
{
    Vec2 result = Vec2(0);
    if (v.x != 0 || v.y != 0) result = v / length(v);
    return result;
}
inline bool equals(Vec2 vectorA, Vec2 vectorB)
{
    float32 deltaX = fabs(vectorA.x - vectorB.x);
    float32 deltaY = fabs(vectorA.y - vectorB.y);

    return deltaX < EPSILON && deltaY < EPSILON;
}
inline Vec2 componentwiseMin(Vec2 v1, Vec2 v2)
{
    Vec2 result = Vec2(fmin(v1.x, v2.x), fmin(v1.y, v2.y));
    return result;
}
inline Vec2 componentwiseMax(Vec2 v1, Vec2 v2)
{
    Vec2 result = Vec2(fmax(v1.x, v2.x), fmax(v1.y, v2.y));
    return result;
}


// Vec3
inline float32 dot(Vec3 vectorA, Vec3 vectorB)
{
    float32 result =
        vectorA.x * vectorB.x +
        vectorA.y * vectorB.y +
        vectorA.z * vectorB.z;
    
    return result;
}
inline Vec3 cross(Vec3 vectorA, Vec3 vectorB)
{
    Vec3 result;

    result.x = vectorA.y * vectorB.z - vectorA.z * vectorB.y;
    result.y = vectorA.z * vectorB.x - vectorA.x * vectorB.z;
    result.z = vectorA.x * vectorB.y - vectorA.y * vectorB.x;

    return result;
}
inline Vec3 hadamard(Vec3 v1, Vec3 v2)
{
    Vec3 result = Vec3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
    return result;
}
inline Vec3 hadamardDivide(Vec3 v1, Vec3 v2)
{
    Vec3 result = Vec3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
    return result;
}
inline float32 lengthSquared(Vec3 v)
{
    float32 result = v.x * v.x + v.y * v.y + v.z * v.z;
    return result;
}
inline float32 length(Vec3 v)
{
    float32 result = sqrtf(lengthSquared(v));
    return result;
}
inline Vec3 normalize(Vec3 v)
{
    Vec3 result = v / length(v);
    return result;
}
inline Vec3 normalizeOrZero(Vec3 v)
{
    Vec3 result = Vec3(0);
    if (v.x != 0 || v.y != 0 || v.z != 0) result = v / length(v);
    return result;
}
inline bool equals(Vec3 vectorA, Vec3 vectorB)
{
    float32 deltaX = fabs(vectorA.x - vectorB.x);
    float32 deltaY = fabs(vectorA.y - vectorB.y);
    float32 deltaZ = fabs(vectorA.z - vectorB.z);

    return deltaX < EPSILON && deltaY < EPSILON && deltaZ < EPSILON;
}
inline Vec3 componentwiseMin(Vec3 v1, Vec3 v2)
{
    Vec3 result = Vec3(fmin(v1.x, v2.x), fmin(v1.y, v2.y), fmin(v1.z, v2.z));
    return result;
}
inline Vec3 componentwiseMax(Vec3 v1, Vec3 v2)
{
    Vec3 result = Vec3(fmax(v1.x, v2.x), fmax(v1.y, v2.y), fmax(v1.z, v2.z));
    return result;
}

// Vec4
inline float32 dot(Vec4 vectorA, Vec4 vectorB)
{
    float32 result =
        vectorA.x * vectorB.x +
        vectorA.y * vectorB.y +
        vectorA.z * vectorB.z +
        vectorA.w * vectorB.w;
    
    return result;
}
inline Vec4 hadamard(Vec4 v1, Vec4 v2)
{
    Vec4 result = Vec4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
    return result;
}
inline Vec4 hadamardDivide(Vec4 v1, Vec4 v2)
{
    Vec4 result = Vec4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
    return result;
}
inline float32 lengthSquared(Vec4 v)
{
    float32 result = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    return result;
}
inline float32 length(Vec4 v)
{
    float32 result = sqrtf(lengthSquared(v));
    return result;
}
inline Vec4 normalize(Vec4 v)
{
    Vec4 result = v / length(v);
    return result;
}
inline Vec4 normalizeOrZero(Vec4 v)
{
    Vec4 result = Vec4(0);
    if (v.x != 0 || v.y != 0 || v.z != 0 || v.w != 0) result = v / length(v);
    return result;
}
inline bool equals(Vec4 vectorA, Vec4 vectorB)
{
    float32 deltaX = fabs(vectorA.x - vectorB.x);
    float32 deltaY = fabs(vectorA.y - vectorB.y);
    float32 deltaZ = fabs(vectorA.z - vectorB.z);
    float32 deltaW = fabs(vectorA.w - vectorB.w);

    return deltaX < EPSILON && deltaY < EPSILON && deltaZ < EPSILON && deltaW < EPSILON;
}
inline Vec4 componentwiseMin(Vec4 v1, Vec4 v2)
{
    Vec4 result = Vec4(fmin(v1.x, v2.x), fmin(v1.y, v2.y), fmin(v1.z, v2.z), fmin(v1.w, v2.w));
    return result;
}
inline Vec4 componentwiseMax(Vec4 v1, Vec4 v2)
{
    Vec4 result = Vec4(fmax(v1.x, v2.x), fmax(v1.y, v2.y), fmax(v1.z, v2.z), fmax(v1.w, v2.w));
    return result;
}

// Quat
inline float32 lengthSquared(Quaternion q)
{
    float32 result = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    return result;
}
inline float32 length(Quaternion q)
{
    float32 result = sqrtf(lengthSquared(q));
    return result;
}
inline Quaternion inverse(Quaternion q)
{
    return Quaternion(-q.x, -q.y, -q.z, q.w);
}
inline Quaternion normalize(Quaternion q)
{
    Quaternion result = q / length(q);
    return result;
}
inline Quaternion relativeRotation(Quaternion start, Quaternion end)
{
    // NOTE: Returns the quaternion needed to transition something from orientation 'start'
    //       to orientation 'end'
    Quaternion result;

    result = end * inverse(start);

    return result;
}
inline Quaternion positiveW(Quaternion q)
{
    if (q.w > 0) return q;
    return -q;
}

///////////////////////////////////////////
//             X-Y-Z Tait-Bryan          //
Vec3 toEuler(Mat4 m);                    //
Vec3 toEuler(Quaternion q);              //
Quaternion fromEuler(Vec3 euler);        //
///////////////////////////////////////////

Mat4 lookAt(Vec3 position, Vec3 target, Vec3 up);
Quaternion relativeRotation(Vec3 start, Vec3 end);
Quaternion lookRotation(Vec3 forward, Vec3 up);


inline Vec3 someNonParallelAxis(Vec3 vector)
{
    if (!FLOAT_EQ(vector.x, 0, EPSILON))
    {
        return Vec3(0, 0, -1);
    }

    if (!FLOAT_EQ(vector.y, 0, EPSILON))
    {
        return Vec3(0, 0, -1);
    }

    if (!FLOAT_EQ(vector.z, 0, EPSILON))
    {
        return Vec3(1, 0, 0);
    }

    assert(false);
    return Vec3(1, 0, 0);
}

inline bool equals(Quaternion quatA, Quaternion quatB)
{
    float32 deltaX = fabs(quatA.x - quatB.x);
    float32 deltaY = fabs(quatA.y - quatB.y);
    float32 deltaZ = fabs(quatA.z - quatB.z);
    float32 deltaW = fabs(quatA.w - quatB.w);

    return deltaX < EPSILON && deltaY < EPSILON && deltaZ < EPSILON && deltaW < EPSILON;
}


//=== Slightly more advanced functions ===//

inline Vec3 project(Vec3 vectorA, Vec3 vectorB)
{
    // Note: project vectorA ONTO vectorB
    Vec3 result;

    float32 dotValue = dot(vectorA, vectorB);
    result = (vectorB * dotValue) / lengthSquared(vectorB);
    
    return result;
}

bool isNormal(Vec3);

// Note: negative distance means BEHIND the plane
inline float32 signedDistance(Vec3 vector, Plane plane)
{
    Vec3 planeToVec = vector - plane.point;
    
    float32 result = dot(planeToVec, plane.normal);
    
    return result;
}

inline float32 distance(Vec3 vector, Plane plane)
{
    float32 result = fabsf(signedDistance(vector, plane));
    return result;
}

inline float32 distance(Vec2 vectorA, Vec2 vectorB)
{
    float32 result = length(vectorA - vectorB);
    return result;
}

inline float32 distanceSquared(Vec2 vectorA, Vec2 vectorB)
{
    float32 result = lengthSquared(vectorA - vectorB);
    return result;
}

inline float32 distance(Vec3 vectorA, Vec3 vectorB)
{
    float32 result = length(vectorA - vectorB);
    return result;
}

inline float32 distanceSquared(Vec3 vectorA, Vec3 vectorB)
{
    float32 result = lengthSquared(vectorA - vectorB);
    return result;
}

inline float32 distance(Vec4 vectorA, Vec4 vectorB)
{
    float32 result = length(vectorA - vectorB);
    return result;
}

inline float32 distanceSquared(Vec4 vectorA, Vec4 vectorB)
{
    float32 result = lengthSquared(vectorA - vectorB);
    return result;
}

// inline bool directionEquals(Vec3 vectorA, Vec3 vectorB)
// {
//     vectorA = normalize(vectorA);
//     vectorB = normalize(vectorB);
    
//     return equals(vectorA, vectorB);
// }

inline bool isZeroVector(Vec2 v)
{
    bool result = (v.x == 0) && (v.y == 0);
    
    return result;
}

inline bool isZeroVector(Vec3 v)
{
    bool result = (v.x == 0) && (v.y == 0) && (v.z == 0);
    
    return result;
}

inline bool isZeroVector(Vec4 v)
{
    bool result = (v.x == 0) && (v.y == 0) && (v.z == 0) && (v.w == 0);
    
    return result;
}

inline bool isNormal(Vec2 v)
{
    bool result;
    result = FLOAT_EQ(lengthSquared(v), 1.0f, 0.01);

    return result;
}

inline bool isNormal(Vec3 v)
{
    bool result;
    result = FLOAT_EQ(lengthSquared(v), 1.0f, 0.01);

    return result;
}

inline bool isNormal(Vec4 v)
{
    bool result;
    result = FLOAT_EQ(lengthSquared(v), 1.0f, 0.01);
    
    return result;
}

inline Vec3 project(Vec3 vector, Plane plane)
{
    assert(isNormal(plane.normal));
    
    Vec3 vProjNormal = project(vector, plane.normal);
    Vec3 result = vector - vProjNormal;

    return result;
}

inline bool isOrthogonal(Vec3 vectorA, Vec3 vectorB)
{
    assert(!isZeroVector(vectorA) && !isZeroVector(vectorB));
    float32 result = fabs(dot(vectorA, vectorB));

    /* return result < .0001; */
    return result < .02; // WHYYYYYYYYYYY
}

inline bool isCollinear(Vec3 pointA, Vec3 pointB, Vec3 pointC)
{
    Vec3 ab = pointB - pointA;
    Vec3 ac = pointC - pointA;
    
    // We will check dx/dz and dy/dz -- and if they are the same from
    // A -> B and A -> C, then the points are collinear. However,
    // there is 1 caveat.  If all the points have the same z value,
    // then we will get divide by 0 errors.  In this situation, we
    // just need to check dy/dx, i.e., the slope of the lines, since
    // they are on the same plane.
    if(fabs(ab.z) < EPSILON)
    {
        if(fabs(ac.z) < EPSILON)
        {
            // 2D collinear test! Same caveat where delta X may be 0,
            // so check for that first.

            if(fabs(ab.x) < EPSILON || fabs(ac.x) < EPSILON)
            {
                if(fabs(ab.x) < EPSILON && fabs(ac.x) < EPSILON)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            // All points have different X values
            float32 abDyDx = ab.y / ab.x;
            float32 acDyDx = ac.y / ac.x;

            return FLOAT_EQ(abDyDx, acDyDx, EPSILON);
        }
        else
        {
            return false;
        }
    }

    // All points have different Z values
    float32 abDxDz = ab.x / ab.z;
    float32 abDyDz = ab.y / ab.z;

    float32 acDxDz = ac.x / ac.z;
    float32 acDyDz = ac.y / ac.z;

    return FLOAT_EQ(abDxDz, acDxDz, EPSILON) && FLOAT_EQ(abDyDz, acDyDz, EPSILON);
}

// http://answers.unity3d.com/questions/372371/multiply-quaternion-by-vector3-how-is-done.html
inline Mat3 rotationMatrix3(Quaternion q)
{
    Mat3 result;

    result[0][0] = 1 - (2 * q.y * q.y) - (2 * q.z * q.z);
    result[0][1] = 2 * q.x * q.y - 2 * q.z * q.w;
    result[0][2] = 2 * q.x * q.z + 2 * q.y * q.w;

    result[1][0] = 2 * q.x * q.y + 2 * q.z * q.w;
    result[1][1] = 1 - (2 * q.x * q.x) - (2 * q.z * q.z);
    result[1][2] = 2 * q.y * q.z - 2 * q.x * q.w;

    result[2][0] = 2 * q.x * q.z - 2 * q.y * q.w;
    result[2][1] = 2 * q.y * q.z + 2 * q.x * q.w;
    result[2][2] = 1 - (2 * q.x * q.x) - (2 * q.y * q.y);

    return result;
}

inline Mat4 rotationMatrix4(Quaternion q)
{
    Mat4 result;
    Mat3 m = rotationMatrix3(q);

    result[0][0] = m[0][0];
    result[0][1] = m[0][1];
    result[0][2] = m[0][2];
    result[0][3] = 0;
    
    result[1][0] = m[1][0];
    result[1][1] = m[1][1];
    result[1][2] = m[1][2];
    result[1][3] = 0;
    
    result[2][0] = m[2][0];
    result[2][1] = m[2][1];
    result[2][2] = m[2][2];
    result[2][3] = 0;

    result[3][0] = 0;
    result[3][1] = 0;
    result[3][2] = 0;
    result[3][3] = 1;
    
    return result;
}

inline Quaternion identityQuaternion()
{
    Quaternion q;
    q.x = q.y = q.z = 0;
    q.w = 1;

    return q;
}

inline Quaternion axisAngle(Vec3 axis, float32 degrees)
{
    axis = normalize(axis);
    
    Quaternion q;
    float32 sinHalfAngle = sinf(TO_RAD(degrees / 2.0f));
    
    q.x = axis.x * sinHalfAngle;
    q.y = axis.y * sinHalfAngle;
    q.z = axis.z * sinHalfAngle;
    q.w = cosf(TO_RAD(degrees / 2.0f));

    return q;
}

inline Quaternion rotateVector(Vec3 initial, Vec3 target)
{
    Quaternion result;

    Vec3 initialNormal = normalize(initial);
    Vec3 targetNormal = normalize(target);

    if (equals(initialNormal, targetNormal)) {
        return identityQuaternion();
    }

    Vec3 axis = cross(initial, target);
    float32 angle = TO_DEG(
        acos(
            dot(
                normalize(initial),
                normalize(target)
            )
        )
    );
    
    result = axisAngle(axis, angle);

    return result;
}

inline Mat3 transposeOf(Mat3 m)
{
    Mat3 result;
    
    result[0][0] = m[0][0];
    result[0][1] = m[1][0];
    result[0][2] = m[2][0];

    result[1][0] = m[0][1];
    result[1][1] = m[1][1];
    result[1][2] = m[2][1];

    result[2][0] = m[0][2];
    result[2][1] = m[1][2];
    result[2][2] = m[2][2];

    return result;
}

inline Mat4 transposeOf(Mat4 m)
{
    Mat4 result;
    
    result[0][0] = m[0][0];
    result[0][1] = m[1][0];
    result[0][2] = m[2][0];
    result[0][3] = m[3][0];

    result[1][0] = m[0][1];
    result[1][1] = m[1][1];
    result[1][2] = m[2][1];
    result[1][3] = m[3][1];

    result[2][0] = m[0][2];
    result[2][1] = m[1][2];
    result[2][2] = m[2][2];
    result[2][3] = m[3][2];

    result[3][0] = m[0][3];
    result[3][1] = m[1][3];
    result[3][2] = m[2][3];
    result[3][3] = m[3][3];

    return result;
}

inline Mat4 translationMatrix(Vec3 translation)
{
    Mat4 result = {};
    
    result[0][3] = translation.x;
    result[1][3] = translation.y;
    result[2][3] = translation.z;

    result[0][0] = 1;
    result[1][1] = 1;
    result[2][2] = 1;
    result[3][3] = 1;

    return result;
}

inline Mat4 scalingMatrix(Vec3 scale)
{
    Mat4 result = {};

    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;
    result[3][3] = 1;

    return result;
}

inline float32 getValue(Vec3 vector, Axis3D axis)
{
    return vector.element[(uint32)axis];
}

//
// Random utilities
//
inline bool isBetween(float32 value, float32 bound1, float32 bound2)
{
    if (value >= bound1 && value <= bound2) return true;
    if (value >= bound2 && value <= bound1) return true;
    return false;
}

inline float32 clamp(float32 value, float32 min, float32 max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

inline float32 maxAbs(float32 value1, float32 value2)
{
    // Returns the (actual) value of the number with the greatest absolute value

    if (fabs(value1) > fabs(value2)) return value1;
    return value2;
}

inline Vec3 barycentricCoordinate(Vec2 a, Vec2 b, Vec2 c, Vec2 point)
{
    // Implementation using Cramer's Rule from Real-Time Collision Detection textbook
    Vec2 v0 = b - a;
    Vec2 v1 = c - a;
    Vec2 v2 = point - a;
    
    float32 d00 = dot(v0, v0);
    float32 d01 = dot(v0, v1);
    float32 d11 = dot(v1, v1);
    float32 d20 = dot(v2, v0);
    float32 d21 = dot(v2, v1);
    float32 denom = d00 * d11 - d01 * d01;

    Vec3 result;

    result.y = (d11 * d20 - d01 * d21) / denom;
    result.z = (d00 * d21 - d01 * d20) / denom;
    result.x = 1.0f - result.y - result.z;

    return result;
}

inline Vec3 triangleNormal(Triangle& triangle)
{
    Vec3 ab = triangle.b - triangle.a;
    Vec3 ac = triangle.c - triangle.a;
    Vec3 n = cross(ab, ac);

    if(n.x == 0 && n.y == 0 && n.z == 0)
    {
        // This triangle is literally a single point...
        if(equals(triangle.a, triangle.b) && equals(triangle.a, triangle.c))
        {
            assert(false);
            n = Vec3(1, 0, 0);
        }
			
        // Isn't a true triangle, since ab and ac are parallel...
        // Simply return a vector that is parallel to bc
        if(equals(triangle.b, triangle.c))
        {
            // AB and AC are same
            Vec3 nonParallel = ab + Vec3(1, 0, 0);

            n = cross(ab, nonParallel);

            assert(n.x != 0 || n.y != 0 || n.z != 0);
        }
        else
        {
            // AB and AC are in opposite directions
            Vec3 bc = triangle.c - triangle.b;
            Vec3 nonParallel = bc + Vec3(1, 0, 0);

            n = cross(bc, nonParallel);

            assert(n.x != 0 || n.y != 0 || n.z != 0);
        }
    }
		
    n.normalizeInPlace();
		
    return n;
}

inline void rewind(Triangle* t)
{
    Vec3 temp;
    temp = t->b;
    t->b = t->c;
    t->c = temp;
}
