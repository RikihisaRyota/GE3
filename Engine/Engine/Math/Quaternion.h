#pragma once

#include "Vector3.h"

struct Quaternion {
    static const Quaternion identity;
    float x;
    float y;
    float z;
    float w;

    inline constexpr Vector3 GetRight() const noexcept {
        float yw = y * w, zw = z * w;
        float yx = y * x, zx = z * x;
        return {
             w * w + x * x - y * y - z * z,
             yx + yx + zw + zw,
             zx + zx - yw - yw,
        };
    }
    inline constexpr Vector3 GetUp() const noexcept {
        float xw = x * w, zw = z * w;
        float xy = x * y, zy = z * y;
        return {
             xy + xy - zw - zw,
             w * w - x * x + y * y - z * z,
             zy + zy + xw + xw,
        };
    }
    inline constexpr Vector3 GetForward() const noexcept {
        float xw = x * w, yw = y * w;
        float xz = x * z, yz = y * z;
        return {
             xz + xz + yw + yw,
             yz + yz - xw - xw,
             w * w - x * x - y * y + z * z,
        };
    }

    // 乗算演算子のオーバーロード
    Quaternion operator*(const Quaternion& rhs) const {
        return Quaternion{
            w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
            w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
            w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
            w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z };
    }

    // 加算演算子のオーバーロード
    Quaternion operator+(const Quaternion& rhs) const {
        return Quaternion{ x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w };
    }

    // 減算演算子のオーバーロード
    Quaternion operator-(const Quaternion& rhs) const {
        return Quaternion{ x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
    }

    // 除算演算子のオーバーロード
    Quaternion operator/(float scalar) const {
        return Quaternion{ x / scalar, y / scalar, z / scalar, w / scalar };
    }

    // Quaternion * float のオーバーロード
    Quaternion operator*(float scalar) const {
        return Quaternion{ x * scalar, y * scalar, z * scalar, w * scalar };
    }

    // Inverse of the quaternion
    Quaternion Inverse() const {
        float norm = x * x + y * y + z * z + w * w;
        return Quaternion{ -x / norm, -y / norm, -z / norm, w / norm };
    }

    // Quaternion * Vector3 のオーバーロード
    Vector3 operator*(const Vector3& vec) const {
        Quaternion vecQuat{ vec.x, vec.y, vec.z, 0.0f };
        Quaternion resQuat = (*this) * vecQuat * Inverse();
        return Vector3{ resQuat.x, resQuat.y, resQuat.z };
    }
};
