#include "Vec3.h"

Vec3::Vec3() : R(0.0f), G(0.0f), B(0.0f) {
}

Vec3::Vec3(float _RGB) : R(_RGB), G(_RGB), B(_RGB) {
}

Vec3::Vec3(float _R, float _G, float _B) : R(_R), G(_G), B(_B) {
}

Vec3 Vec3::operator+(const Vec3& _Other) const {
	return Vec3(R + _Other.R, G + _Other.G, B + _Other.B);
}

Vec3 Vec3::operator-(const Vec3& _Other) const {
	return Vec3(R - _Other.R, G - _Other.G, B - _Other.B);
}

Vec3 Vec3::operator+(float _Scalar) const {
	return Vec3(R + _Scalar, G + _Scalar, B + _Scalar);
}

Vec3 Vec3::operator-(float _Scalar) const {
	return Vec3(R - _Scalar, G - _Scalar, B - _Scalar);
}
