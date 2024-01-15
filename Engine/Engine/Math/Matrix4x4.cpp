#include "Matrix4x4.h"

#include <cassert>
#include <cmath>


#include "Vector3.h"
#include "MyMath.h"

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& m2) {
	Matrix4x4 mat;
	mat.m[0][0] = this->m[0][0] * m2.m[0][0] + this->m[0][1] * m2.m[1][0] + this->m[0][2] * m2.m[2][0] +
		this->m[0][3] * m2.m[3][0],
		mat.m[0][1] = this->m[0][0] * m2.m[0][1] + this->m[0][1] * m2.m[1][1] + this->m[0][2] * m2.m[2][1] +
		this->m[0][3] * m2.m[3][1],
		mat.m[0][2] = this->m[0][0] * m2.m[0][2] + this->m[0][1] * m2.m[1][2] + this->m[0][2] * m2.m[2][2] +
		this->m[0][3] * m2.m[3][2],
		mat.m[0][3] = this->m[0][0] * m2.m[0][3] + this->m[0][1] * m2.m[1][3] + this->m[0][2] * m2.m[2][3] +
		this->m[0][3] * m2.m[3][3],

		mat.m[1][0] = this->m[1][0] * m2.m[0][0] + this->m[1][1] * m2.m[1][0] + this->m[1][2] * m2.m[2][0] +
		this->m[1][3] * m2.m[3][0],
		mat.m[1][1] = this->m[1][0] * m2.m[0][1] + this->m[1][1] * m2.m[1][1] + this->m[1][2] * m2.m[2][1] +
		this->m[1][3] * m2.m[3][1],
		mat.m[1][2] = this->m[1][0] * m2.m[0][2] + this->m[1][1] * m2.m[1][2] + this->m[1][2] * m2.m[2][2] +
		this->m[1][3] * m2.m[3][2],
		mat.m[1][3] = this->m[1][0] * m2.m[0][3] + this->m[1][1] * m2.m[1][3] + this->m[1][2] * m2.m[2][3] +
		this->m[1][3] * m2.m[3][3],

		mat.m[2][0] = this->m[2][0] * m2.m[0][0] + this->m[2][1] * m2.m[1][0] + this->m[2][2] * m2.m[2][0] +
		this->m[2][3] * m2.m[3][0],
		mat.m[2][1] = this->m[2][0] * m2.m[0][1] + this->m[2][1] * m2.m[1][1] + this->m[2][2] * m2.m[2][1] +
		this->m[2][3] * m2.m[3][1],
		mat.m[2][2] = this->m[2][0] * m2.m[0][2] + this->m[2][1] * m2.m[1][2] + this->m[2][2] * m2.m[2][2] +
		this->m[2][3] * m2.m[3][2],
		mat.m[2][3] = this->m[2][0] * m2.m[0][3] + this->m[2][1] * m2.m[1][3] + this->m[2][2] * m2.m[2][3] +
		this->m[2][3] * m2.m[3][3],

		mat.m[3][0] = this->m[3][0] * m2.m[0][0] + this->m[3][1] * m2.m[1][0] + this->m[3][2] * m2.m[2][0] +
		this->m[3][3] * m2.m[3][0],
		mat.m[3][1] = this->m[3][0] * m2.m[0][1] + this->m[3][1] * m2.m[1][1] + this->m[3][2] * m2.m[2][1] +
		this->m[3][3] * m2.m[3][1],
		mat.m[3][2] = this->m[3][0] * m2.m[0][2] + this->m[3][1] * m2.m[1][2] + this->m[3][2] * m2.m[2][2] +
		this->m[3][3] * m2.m[3][2],
		mat.m[3][3] = this->m[3][0] * m2.m[0][3] + this->m[3][1] * m2.m[1][3] + this->m[3][2] * m2.m[2][3] +
		this->m[3][3] * m2.m[3][3];

	return mat;
}

Matrix4x4 Matrix4x4::operator*=(const Matrix4x4& mat) {
	// 行列の乗算を実行し、結果を格納する
	Matrix4x4 result = MakeIdentity4x4();

	for (int row = 0; row < 4; ++row) {
		for (int col = 0; col < 4; ++col) {
			result.m[row][col] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				result.m[row][col] += m[row][k] * mat.m[k][col];
			}
		}
	}

	// 結果を現在の行列に格納する
	*this = result;

	return *this;
}

Vector3 Matrix4x4::operator*(const Vector3& mat) { return Transform(mat, *this); }
