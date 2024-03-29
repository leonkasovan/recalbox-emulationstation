#pragma once
#ifndef ES_CORE_MATH_TRANSFORM4X4F_H
#define ES_CORE_MATH_TRANSFORM4X4F_H

#include "Vector4f.h"
#include "Vector3f.h"

class Transform4x4f
{
public:

	Transform4x4f() = default;
	Transform4x4f(const Vector4f& _r0, const Vector4f& _r1, const Vector4f& _r2, const Vector4f& _r3) : mR0(_r0), mR1(_r1), mR2(_r2), mR3(_r3) { }

	Transform4x4f  operator* (const Transform4x4f& _other) const;
	Vector3f operator* (const Vector3f& _other) const;
	Transform4x4f& operator*=(const Transform4x4f& _other) { *this = *this * _other; return *this; }

	inline       Vector4f& r0()       { return mR0; }
	inline       Vector4f& r1()       { return mR1; }
	inline       Vector4f& r2()       { return mR2; }
	inline       Vector4f& r3()       { return mR3; }
	inline const Vector4f& r0() const { return mR0; }
	inline const Vector4f& r1() const { return mR1; }
	inline const Vector4f& r2() const { return mR2; }
	inline const Vector4f& r3() const { return mR3; }

	Transform4x4f& invert   (const Transform4x4f& _other);
	Transform4x4f& scale    (const Vector3f& _scale);
	Transform4x4f& rotate   (float _angle, const Vector3f& _axis);
	Transform4x4f& rotateX  (float _angle);
	Transform4x4f& rotateY  (float _angle);
	Transform4x4f& rotateZ  (float _angle);
	Transform4x4f& translate(const Vector3f& _translation);
  Transform4x4f& translate(float dx, float dy);
	Transform4x4f& round    ();

	inline       Vector3f& translation()       { return mR3.v3(); }
	inline const Vector3f& translation() const { return mR3.v3(); }

	static       Transform4x4f Identity() { return { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } }; }

protected:

	Vector4f mR0;
	Vector4f mR1;
	Vector4f mR2;
	Vector4f mR3;

}; // Transform4x4f

#endif // ES_CORE_MATH_TRANSFORM4X4F_H
