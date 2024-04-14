#pragma once
struct Quaternion {
	static const Quaternion identity;
	float x;
	float y;
	float z;
	float w;

    // 乗算演算子のオーバーロード
    Quaternion operator*(const Quaternion& rhs) const {
        return Quaternion{
            w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
            w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
            w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
            w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
        };
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
};
