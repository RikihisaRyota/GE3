#pragma once
struct Vector3;
struct Matrix4x4
{
	float m[4][4];

	Matrix4x4 operator*(const Matrix4x4& mat);
	Matrix4x4 operator*=(const Matrix4x4& mat);
	Vector3 operator*(const Vector3& mat);
};