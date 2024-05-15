#include "Vector3.h"

#include <assert.h>

#include "Engine/Math/Matrix4x4.h"


Vector3 Vector3::operator*(const Matrix4x4& mat) const {
	return {
			  this->x * mat.m[0][0] + this->y * mat.m[1][0] + this->z * mat.m[2][0] + mat.m[3][0],
			  this->x * mat.m[0][1] + this->y * mat.m[1][1] + this->z * mat.m[2][1] + mat.m[3][1],
			  this->x * mat.m[0][2] + this->y * mat.m[1][2] + this->z * mat.m[2][2] + mat.m[3][2] };
}
