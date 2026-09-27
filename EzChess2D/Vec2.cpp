#include "Vec2.h"

Vec2::Vec2() : X(0.0f), Y(0.0f) {
}

Vec2::Vec2(float _XY) : X(_XY), Y(_XY) {
}

Vec2::Vec2(float _X, float _Y) : X(_X), Y(_Y) {
}

Vec2 Vec2::operator+(const Vec2& _Other) const {
	return Vec2(X + _Other.X, Y + _Other.Y);
}

Vec2 Vec2::operator-(const Vec2& _Other) const {
	return Vec2(X - _Other.X, Y - _Other.Y);
}

Vec2 Vec2::operator+(float _Scalar) const {
	return Vec2(X + _Scalar, Y + _Scalar);
}

Vec2 Vec2::operator-(float _Scalar) const {
	return Vec2(X - _Scalar, Y - _Scalar);
}

bool Vec2::operator==(const Vec2& _Other) const {
	return X == _Other.X && Y == _Other.Y;
}

bool Vec2::operator!=(const Vec2& _Other) const {
	return !(*this == _Other);
}
