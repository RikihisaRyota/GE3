#pragma once
/**
 * @file Vector4.h
 */

#include <cmath>

struct Vector4 {
    float x;
    float y;
    float z;
    float w;

    // コンストラクタ
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    // デフォルトコンストラクタ
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

    // + 演算子のオーバーロード
    Vector4 operator+(const Vector4& other) const {
        return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    // - 演算子のオーバーロード
    Vector4 operator-(const Vector4& other) const {
        return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    // * 演算子のオーバーロード（スカラー倍）
    Vector4 operator*(float scalar) const {
        return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    // * 演算子のオーバーロード（要素ごとの積）
    Vector4 operator*(const Vector4& other) const {
        return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
    }

    // == 演算子のオーバーロード
    bool operator==(const Vector4& other) const {
        return x == other.x && y == other.y && z == other.z && w == other.w;
    }

    // != 演算子のオーバーロード
    bool operator!=(const Vector4& other) const {
        return !(*this == other);
    }

    // *= 演算子のオーバーロード（スカラー倍）
    Vector4& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    // ベクトルの長さを計算する関数
    float Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
};