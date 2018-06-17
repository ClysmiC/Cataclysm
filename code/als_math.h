#pragma once

#include "math.h"
#include "assert.h"
#include "Types.h"

#define FLOAT_EQ(x, y, epsilon) (fabs((x) - (y)) < epsilon)
#define PI 3.14159265359f
#define EPSILON 0.0001f
#define TO_RAD(x) ((x)* PI / 180.0f)
#define TO_DEG(x) ((x)* 180.0f/ PI)

enum Axis3D
{
	X = 0,
	Y = 1,
	Z = 2
};


union Vec2
{
	Vec2();
	Vec2(real32 value);
	Vec2(real32 x, real32 y);
	
    struct
    {
        real32 x, y;
    };
    real32 element[2];

	Vec2& normalizeOrXAxisInPlace();
	Vec2& normalizeInPlace();
};

union Vec3
{
	Vec3();
	Vec3(Axis3D unitAxis);
	Vec3(real32 value);
	Vec3(Vec2 xy, real32 z);
	Vec3(real32 x, real32 y, real32 z);
	
    struct
    {
        real32 x, y, z;
    };
    struct
    {
        real32 r, g, b;
    };
    real32 element[3];

	Vec3& normalizeOrXAxisInPlace();
	Vec3& normalizeInPlace();
};

union Vec4
{
	Vec4();
	Vec4(real32 value);
	Vec4(Vec2 xy, real32 z, real32 w);
	Vec4(Vec2 xy, Vec2 zw);
	Vec4(Vec3 xyz, real32 w);
	Vec4(real32 x, real32 y, real32 z, real32 w);
	
    struct
    {
        real32 x, y, z, w;
    };
    struct
    {
        real32 r, g, b, a;
    };
    real32 element[4];

	Vec4& normalizeInPlace();
};

union Quaternion
{
	Quaternion();
	Quaternion(Vec3 vector, real32 scalar);
	Quaternion(Vec4 xyzw);
	Quaternion(real32 x, real32 y, real32 z, real32 w);
	
    struct
    {
        real32 x, y, z, w;
    };
    real32 element[4];

	Quaternion& normalizeInPlace();
	Quaternion& invertInPlace();
};

struct Mat4;
struct Mat3
{
	// Mat3(Mat4 mat4);
	
	real32 _e[3][3] = {
		{ 1, 0, 0},
		{ 0, 1, 0},
		{ 0, 0, 1}
	};

	// Only overloads the first index to point to the correct array, then
	// the second index is handled by built-in c++
	real32* operator [] (int32 i);
	real32* dataPointer();

	Mat3& transposeInPlace();
};

struct Mat4
{
	// Mat4(Mat3 mat3);
	
	real32 _e[4][4] = {
		{ 1, 0, 0, 0},
		{ 0, 1, 0, 0},
		{ 0, 0, 1, 0},
		{ 0, 0, 0, 1}
	};

	// Only overloads the first index to point to the correct array, then
	// the second index is handled by built-in c++
	real32* operator [] (int32 i);
	real32* dataPointer();

	Mat4& scaleInPlace(Vec3 scale);
	Mat4& rotateInPlace(Quaternion rotation);
	Mat4& translateInPlace(Vec3 translation);
	
	Mat4& transposeInPlace();
	Mat4& mat3ifyInPlace();
	Mat4& perspectiveInPlace(real32 fov, real32 aspectRatio, real32 near, real32 far);
};

struct Plane
{
	Plane(Vec3 point, Vec3 normal);
	Plane(Vec3 pointA, Vec3 pointB, Vec3 pointC); // NOTE: normal calculated using CCW winding
	Vec3 point;
	Vec3 normal;
};

Vec2 operator + (Vec2 vectorA, Vec2 vectorB);
Vec2 operator + (Vec2 vector, real32 constant);
Vec2& operator += (Vec2& vectorA, Vec2 vectorB);
Vec2& operator += (Vec2& vector, real32 constant);

Vec2 operator - (Vec2 vectorA, Vec2 vectorB);
Vec2 operator - (Vec2 vector, real32 constant);
Vec2& operator -= (Vec2& vectorA, Vec2 vectorB);
Vec2& operator -= (Vec2& vector, real32 constant);
Vec2 operator - (Vec2 vector);


Vec2 operator * (real32 scalar, Vec2 vector);
Vec2 operator * (Vec2 vector, real32 scalar);
Vec2& operator *= (Vec2& vector, real32 scalar);

Vec2 operator / (Vec2 vector, real32 scalar);
Vec2& operator /= (Vec2& vector, real32 scalar);


// Vec3
Vec3 operator + (Vec3 vectorA, Vec3 vectorB);
Vec3 operator + (Vec3 vector, real32 constant);
Vec3& operator += (Vec3& vectorA, Vec3 vectorB);
Vec3& operator += (Vec3& vector, real32 constant);

Vec3 operator - (Vec3 vectorA, Vec3 vectorB);
Vec3 operator - (Vec3 vector, real32 constant);
Vec3& operator -= (Vec3& vectorA, Vec3 vectorB);
Vec3& operator -= (Vec3& vector, real32 constant);
Vec3 operator - (Vec3 vector);

Vec3 operator * (real32 scalar, Vec3 vector);
Vec3 operator * (Vec3 vector, real32 scalar);
Vec3& operator *= (Vec3& vector, real32 scalar);

Vec3 operator / (Vec3 vector, real32 scalar);
Vec3& operator /= (Vec3& vector, real32 scalar);

Vec3 operator * (Quaternion quaternion, Vec3 vector);
Vec3 operator * (Mat3 m, Vec3 v);

// Vec4
Vec4 operator + (Vec4 vectorA, Vec4 vectorB);
Vec4 operator + (Vec4 vector, real32 constant);
Vec4& operator += (Vec4& vectorA, Vec4 vectorB);
Vec4& operator += (Vec4& vector, real32 constant);

Vec4 operator - (Vec4 vectorA, Vec4 vectorB);
Vec4 operator - (Vec4 vector, real32 constant);
Vec4& operator -= (Vec4& vectorA, Vec4 vectorB);
Vec4& operator -= (Vec4& vector, real32 constant);
Vec4 operator - (Vec4 vector);

Vec4 operator * (real32 scalar, Vec4 vector);
Vec4 operator * (Vec4 vector, real32 scalar);
Vec4& operator *= (Vec4& vector, real32 scalar);
Vec4 operator / (Vec4 vector, real32 scalar);
Vec4& operator /= (Vec4& vector, real32 scalar);

Vec4 operator * (Mat4 m, Vec4 v);

// Quaternion
Quaternion operator + (Quaternion quatA, Quaternion quatB);
Quaternion operator + (Quaternion quat, real32 constant);
Quaternion& operator += (Quaternion& quatA, Quaternion quatB);
Quaternion& operator += (Quaternion& quat, real32 constant);

Quaternion operator - (Quaternion quatA, Quaternion quatB);
Quaternion operator - (Quaternion quat, real32 constant);
Quaternion& operator -= (Quaternion& quatA, Quaternion quatB);
Quaternion& operator -= (Quaternion& quat, real32 constant);
Quaternion operator - (Quaternion quat);

Quaternion operator * (real32 scalar, Quaternion quat);
Quaternion operator * (Quaternion quat, real32 scalar);
Quaternion& operator *= (Quaternion& quat, real32 scalar);
Quaternion operator / (Quaternion quat, real32 scalar);
Quaternion& operator /= (Quaternion& quat, real32 scalar);

Quaternion operator * (Quaternion a, Quaternion b);
Quaternion lookRotation(Vec3 forward, Vec3 up);

// Mat
Mat3 operator * (Mat3 left, Mat3 right);
Mat4 operator * (Mat4 left, Mat4 right);




//
// FUNCTIONS
//

// Vec2
inline real32 dot(Vec2 vectorA, Vec2 vectorB)
{
	real32 result = vectorA.x * vectorB.x + vectorA.y * vectorB.y;
	return result;
}
inline Vec2 hadamard(Vec2 v1, Vec2 v2)
{
	Vec2 result = Vec2(v1.x * v2.x, v1.y * v2.y);
	return result;
}
inline real32 lengthSquared(Vec2 v)
{
	real32 result = v.x * v.x + v.y * v.y;
	return result;
}
inline real32 length(Vec2 v)
{
	real32 result = sqrtf(lengthSquared(v));
	return result;
}
inline Vec2 normalize(Vec2 v)
{
	Vec2 result = v / length(v);
	return result;
}
inline bool equals(Vec2 vectorA, Vec2 vectorB)
{
	real32 deltaX = fabs(vectorA.x - vectorB.x);
	real32 deltaY = fabs(vectorA.y - vectorB.y);

	return deltaX < EPSILON && deltaY < EPSILON;
}


// Vec3
inline real32 dot(Vec3 vectorA, Vec3 vectorB)
{
	real32 result =
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
inline real32 lengthSquared(Vec3 v)
{
	real32 result = v.x * v.x + v.y * v.y + v.z * v.z;
	return result;
}
inline real32 length(Vec3 v)
{
	real32 result = sqrtf(lengthSquared(v));
	return result;
}
inline Vec3 normalize(Vec3 v)
{
	Vec3 result = v / length(v);
	return result;
}
inline bool equals(Vec3 vectorA, Vec3 vectorB)
{
	real32 deltaX = fabs(vectorA.x - vectorB.x);
	real32 deltaY = fabs(vectorA.y - vectorB.y);
	real32 deltaZ = fabs(vectorA.z - vectorB.z);

	return deltaX < EPSILON && deltaY < EPSILON && deltaZ < EPSILON;
}

// Vec4
inline real32 dot(Vec4 vectorA, Vec4 vectorB)
{
	real32 result =
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
inline real32 lengthSquared(Vec4 v)
{
	real32 result = v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
	return result;
}
inline real32 length(Vec4 v)
{
	real32 result = sqrtf(lengthSquared(v));
	return result;
}
inline Vec4 normalize(Vec4 v)
{
	Vec4 result = v / length(v);
	return result;
}
inline bool equals(Vec4 vectorA, Vec4 vectorB)
{
	real32 deltaX = fabs(vectorA.x - vectorB.x);
	real32 deltaY = fabs(vectorA.y - vectorB.y);
	real32 deltaZ = fabs(vectorA.z - vectorB.z);
	real32 deltaW = fabs(vectorA.w - vectorB.w);

	return deltaX < EPSILON && deltaY < EPSILON && deltaZ < EPSILON && deltaW < EPSILON;
}

// Quat
inline real32 lengthSquared(Quaternion q)
{
	real32 result = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	return result;
}
inline real32 length(Quaternion q)
{
	real32 result = sqrtf(lengthSquared(q));
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
inline bool equals(Quaternion quatA, Quaternion quatB)
{
	real32 deltaX = fabs(quatA.x - quatB.x);
	real32 deltaY = fabs(quatA.y - quatB.y);
	real32 deltaZ = fabs(quatA.z - quatB.z);
	real32 deltaW = fabs(quatA.w - quatB.w);

	return deltaX < EPSILON && deltaY < EPSILON && deltaZ < EPSILON && deltaW < EPSILON;
}


//=== Slightly more advanced functions ===//

inline Vec3 project(Vec3 vectorA, Vec3 vectorB)
{
	Vec3 result;

	real32 dotValue = dot(vectorA, vectorB);
	result = (vectorB * dotValue) / lengthSquared(vectorB);
	
	return result;
}

bool isNormal(Vec3);
inline real32 distanceSquared(Vec3 vector, Plane plane)
{
	assert(isNormal(plane.normal));

	Vec3 toVector = vector - plane.point;
	Vec3 projection = project(toVector, plane.normal);
	real32 result = lengthSquared(projection);
	
	return result;
}

inline real32 distance(Vec3 vector, Plane plane)
{
	real32 result = distanceSquared(vector, plane);
	result = sqrtf(result);
	
	return result;
}

inline real32 distance(Vec2 vectorA, Vec2 vectorB)
{
	real32 result = length(vectorA - vectorB);
	return result;
}

inline real32 distance(Vec3 vectorA, Vec3 vectorB)
{
	real32 result = length(vectorA - vectorB);
	return result;
}

inline real32 distance(Vec4 vectorA, Vec4 vectorB)
{
	real32 result = length(vectorA - vectorB);
	return result;
}

// inline bool directionEquals(Vec3 vectorA, Vec3 vectorB)
// {
// 	vectorA = normalize(vectorA);
// 	vectorB = normalize(vectorB);
	
// 	return equals(vectorA, vectorB);
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
	real32 result = fabs(dot(vectorA, vectorB));

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
			real32 abDyDx = ab.y / ab.x;
			real32 acDyDx = ac.y / ac.x;

			return FLOAT_EQ(abDyDx, acDyDx, EPSILON);
		}
		else
		{
			return false;
		}
	}

	// All points have different Z values
	real32 abDxDz = ab.x / ab.z;
	real32 abDyDz = ab.y / ab.z;

	real32 acDxDz = ac.x / ac.z;
	real32 acDyDz = ac.y / ac.z;

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

inline Quaternion axisAngle(Vec3 axis, real32 degrees)
{
	axis = normalize(axis);
	
	Quaternion q;
	real32 sinHalfAngle = sinf(TO_RAD(degrees / 2.0f));
	
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
	real32 angle = TO_DEG(
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

inline real32 getValue(Vec3 vector, Axis3D axis)
{
	return vector.element[axis];
}
