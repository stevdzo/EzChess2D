#ifndef _VEC_2_H
#define _VEC_2_H

class Vec2 {

public:

	float X, Y;

	Vec2();
	Vec2(float _XY);
	Vec2(float _X, float _Y);

	Vec2 operator+(const Vec2& _Other) const;
	Vec2 operator-(const Vec2& _Other) const;
	Vec2 operator+(float _Scalar) const;
	Vec2 operator-(float _Scalar) const;

	bool operator==(const Vec2& _Other) const;
	bool operator!=(const Vec2& _Other) const;
};
#endif
