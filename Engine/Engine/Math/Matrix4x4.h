#pragma once

#include "Vector3.h"

struct Matrix4x4
{
	float m[4][4];

	Matrix4x4 operator*(const Matrix4x4& mat);
	Matrix4x4 operator*=(const Matrix4x4& mat);
	Vector3 operator*(const Vector3& mat);

    inline constexpr Matrix4x4& SetXAxis(const Vector3& v) noexcept {
        m[0][0] = v.x, m[0][1] = v.y, m[0][2] = v.z;
        return *this;
    }
    inline constexpr Vector3 GetXAxis() const noexcept {
        return { m[0][0], m[0][1], m[0][2] };
    }
    inline constexpr Matrix4x4& SetYAxis(const Vector3& v) noexcept {
        m[1][0] = v.x, m[1][1] = v.y, m[1][2] = v.z;
        return *this;
    }
    inline constexpr Vector3 GetYAxis() const noexcept {
        return { m[1][0], m[1][1], m[1][2] };
    }
    inline constexpr Matrix4x4& SetZAxis(const Vector3& v) noexcept {
        m[2][0] = v.x, m[2][1] = v.y, m[2][2] = v.z;
        return *this;
    }
    inline constexpr Vector3 GetZAxis() const noexcept {
        return { m[2][0], m[2][1], m[2][2] };
    }
    inline constexpr Matrix4x4& SetTranslate(const Vector3& v) noexcept {
        m[3][0] = v.x, m[3][1] = v.y, m[3][2] = v.z;
        return *this;
    }
};