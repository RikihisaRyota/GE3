#pragma once

struct Vector4 {
	float x;
	float y;
	float z;
	float w;
	
	// !=演算子のオーバーロード
	inline bool operator!=(const Vector4& other) const {
		return !(x == other.x && y == other.y && z == other.z && w == other.w);
	}

	// ==演算子のオーバーロード
	inline bool operator==(const Vector4& other) const {
		return (x == other.x && y == other.y && z == other.z && w == other.w);
	}
};