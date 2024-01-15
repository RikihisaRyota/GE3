#pragma once

#include <assert.h>
#include <cmath>

struct Vector2 {
	float x;
	float y;

	Vector2& operator+=(const Vector2& a) {
		this->x += a.x;
		this->y += a.y;
		return *this;
	}

	Vector2& operator-=(const Vector2& a) {
		this->x -= a.x;
		this->y -= a.y;
		return *this;
	}

	Vector2& operator*=(float a) {
		this->x *= a;
		this->y *= a;
		return *this;
	}

	Vector2 operator+(const Vector2& a) const {
		return Vector2(this->x + a.x, this->y + a.y);
	}

	Vector2 operator+(float a) const {
		return Vector2(this->x + a, this->y + a);
	}

	Vector2 operator-(const Vector2& a) const {
		return Vector2(this->x - a.x, this->y - a.y);
	}

	Vector2 operator-(float a) const {
		return Vector2(this->x - a, this->y - a);
	}

	Vector2 operator-() const {
		return Vector2(-this->x, -this->y);
	}

	Vector2 operator*(const Vector2& a) const {
		return Vector2(this->x * a.x, this->y * a.y);
	}

	Vector2 operator*(const float a) const {
		return Vector2(this->x * a, this->y * a);
	}

	bool operator!=(const Vector2& other) const {
		return (x != other.x) || (y != other.y);
	}
};