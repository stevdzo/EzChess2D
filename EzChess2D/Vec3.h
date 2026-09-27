#ifndef _VEC_3_H
#define _VEC_3_H

class Vec3 {

public:

	float R, G, B;

	Vec3();
	Vec3(float _RGB);
	Vec3(float _R, float _G, float _B);

	Vec3 operator+(const Vec3& _Other) const;
	Vec3 operator-(const Vec3& _Other) const;
	Vec3 operator+(float _Scalar) const;
	Vec3 operator-(float _Scalar) const;
};
#endif
