#pragma once
#include <cmath>

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Matrix4x4;
struct Vector3 final {
	float x;
	float y;
	float z;

	// ベクトルの長さを計算する関数
	float Length() const { return std::sqrt(x * x + y * y + z * z); }

	// ベクトルの正規化を行う関数
	Vector3 Normalized() {
		return *this / Length();
	}
	Vector3& Normalize() {
		return (*this = Normalized());
	}
	// 加算演算子のオーバーロード
	Vector3 operator+(const Vector3& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}
	// 加算演算子のオーバーロード
	Vector3 operator+(float other) const {
		return { x + other, y + other, z + other };
	}

	// 減算演算子のオーバーロード
	Vector3 operator-(const Vector3& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}
	// 減算演算子のオーバーロード
	Vector3 operator-(float other) const {
		return { x - other, y - other, z - other };
	}

	// 二項マイナス演算子のオーバーロード（引数なしの単項マイナス演算子ではない）
	Vector3 operator-() const { return { -x, -y, -z }; }

	// 積演算子のオーバーロード
	Vector3 operator*(const Vector3& other) const {
		return { x * other.x, y * other.y, z * other.z };
	}

	// スカラー乗算演算子のオーバーロード
	Vector3 operator*(float scalar) const { return { x * scalar, y * scalar, z * scalar }; }

	// スカラー除算演算子のオーバーロード
	Vector3 operator/(float scalar) const { return { x / scalar, y / scalar, z / scalar }; }

	// +=演算子のオーバーロード
	Vector3& operator+=(const Vector3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	// -=演算子のオーバーロード
	Vector3& operator-=(const Vector3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	// *=演算子のオーバーロード
	Vector3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	// /=演算子のオーバーロード
	Vector3& operator/=(float scalar) {
		x /= scalar;
		y /= scalar;
		z /= scalar;
		return *this;
	}

	// !=演算子のオーバーロード
	bool operator!=(const Vector3& other) const {
		return !(x == other.x && y == other.y && z == other.z);
	}

	// ==演算子のオーバーロード
	bool operator==(const Vector3& other) const {
		return (x == other.x && y == other.y && z == other.z);
	}

	Vector3 operator-() { return Vector3(-x, -y, -z); }

	// Matrix4x4との乗算演算子のオーバーロード
	Vector3 operator*(const Matrix4x4& mat) const;
};