#pragma once
#include <algorithm> // std::min, std::max
#include <vector>

#include "AABB.h"
#include "Matrix4x4.h"
#include "OBB.h"
#include "WorldTransform.h"
#include "ViewProjection.h"
#include "Vector4.h"
#include "Vector2.h"


const double PI = 3.14159265359;

// ベクトル変換
Vector3 TransformNormal(const Vector3& v, const Matrix4x4& m);

Matrix4x4 NotTransform(const Matrix4x4& matrix);

Vector3 Lerp(const Vector3& start, const Vector3& end, float t);
float Lerp(float start, float end, float t);

Vector3 Slerp(const Vector3& start, const Vector3& end, float t);

float Distance(const Vector3& v1, const Vector3& v2);

float Dot(const Vector3& a, const Vector3& b);

Vector3 Subtract(const Vector3& v1, const Vector3& v2);

float Length(const Vector3& a);

Vector3 Normalize(const Vector3& v1);

Vector3 CatmullRom(Vector3 point0, Vector3 point1, Vector3 point2, Vector3 point3, float t);

// 1,行列の加法
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2);
// 2,行列の減算
Matrix4x4 Sub(const Matrix4x4& m1, const Matrix4x4& m2);
// 3,行列の積
Matrix4x4 Mul(const Matrix4x4& m1, const Matrix4x4& m2);
// 3,行列の積(スカラー倍)
Matrix4x4 Mul(const float scaler, const Matrix4x4& m2);
// 4,逆行列
Matrix4x4 Inverse(const Matrix4x4& m);
// 5,転置行列
Matrix4x4 Transpose(const Matrix4x4& m);
// 6,単位行列
Matrix4x4 MakeIdentity4x4();

// 1,平行移動行列
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
// 2,拡大縮小行列
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
// 3,座標変換
Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

// 1,X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian);
// 2,Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian);
// 3,Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian);
//4,全部
Matrix4x4 MakeRotateXYZMatrix(const Vector3& rotation);

// 3次元アフィン変換
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
//1,透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
//2,正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);
//3,ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
// ビュー行列
Matrix4x4 MakeViewMatrix(const Vector3& rotation, const Vector3& translation);
// LookAtLH
Matrix4x4 MakeLookAtLH(const Vector3& target, const Vector3& eye, const Vector3& up);

// Billbord
Matrix4x4 MakeBillboard(const Vector3& target, const Vector3& eye, const Vector3& up);
Matrix4x4 MakeBillboardXAxsizLook(const Vector3& target, const Vector3& eye, const Vector3& up);
Matrix4x4 MakeBillboardYAxsizLook(const Vector3& target, const Vector3& eye, const Vector3& up);
Matrix4x4 MakeBillboardZAxsizLook(const Vector3& target, const Vector3& eye, const Vector3& up);

//1,X軸
Vector3 GetXAxis(const Matrix4x4& mat);
//2,Y軸
Vector3 GetYAxis(const Matrix4x4& mat);
//3,Z軸
Vector3 GetZAxis(const Matrix4x4& mat);
// スクリーン変換行列
Matrix4x4 MakeViewProjectMatrixMatrix(const ViewProjection& viewProjection);

Matrix4x4 Convert(const Matrix4x4& m1);

/// <summary>
/// Clamp
/// </summary>
/// <param name="num"></param>
/// <param name="min">min</param>
/// <param name="max">max</param>
/// <returns></returns>
float Clamp(float num, float min, float max);
/// <summary>
/// radian->degree
/// </summary>
/// <param name="radian">radian</param>
/// <returns>degree</returns>
float RadToDeg(float radian);
/// <summary>
/// degree->radian
/// </summary>
/// <param name="degree">degree</param>
/// <returns>radian</returns>
float DegToRad(float degree);
/// <summary> 
/// ワールドトランスフォームからワールド行列を生成
/// </summary>
/// /// <param name="worldtransform"></param>
/// /// <returns></returns>
Matrix4x4 MakeMatWolrd(const WorldTransform& worldtransform);

/// <summary>
/// ヒットボックス表示
/// </summary>
/// <param name="worldtransform">worldtransform</param>
/// <param name="viewProjection">viewProjection</param>
/// <param name="color">color</param>
//void ChackHitBox(
//    const WorldTransform& worldtransform, const ViewProjection& viewProjection,
//    const Vector4& color);

/// <summary> 
/// 最短角度補間
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <param name="t">t</param>
/// <returns></returns>
float LenpShortAngle(float a, float b, float t);
Vector3 LenpShortAngle(const Vector3& a, const Vector3& b, float t);

Vector3 Perpendicular(const Vector3& vector);

// 外積
// 外積
Vector3 Cross(const Vector3& v1, const Vector3& v2);

// AABBに値を代入
AABB AABBAssignment(const AABB& aabb);

// OBBの回転角度の抽出
OBB OBBSetRotate(const OBB& Obb, const Vector3& rotate);
OBB OBBSetRotate(const OBB& Obb, const Vector3& rotate, const Vector3& rotate1);

// OBBの平行移動
Matrix4x4 OBBMakeWorldMatrix(const OBB& obb);

Matrix4x4 SetRotate(const Vector3 (&array)[3]);

// 曲線
// 二次ベジエ
Vector2 QuadraticBezier(
    const Vector2& controlPoint0, const Vector2& controlPoint1, const Vector2& controlPoint2,
    float t);
// 三次ベジエ
Vector3 CubicBezier(
    const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2,
    float t);

// Lerp
// float Lerp(float start, float end, float t);
// Vector2 Lerp(const Vector2& start, const Vector2& end, float t);
// Vector3 Lerp(const Vector3& start, const Vector3& end, float t);
template<class T> T Lerp(const T& start, const T& end, float t);
template<class T> inline T Lerp(const T& start, const T& end, float t) {
	return start + (end - start) * t;
}

// CatmullRom
Vector2 QuadraticCatmullRom(
    const Vector2& Position0, const Vector2& Position1, const Vector2& Position2,
    const Vector2& Position3, float t);
Vector3 CubicCatmullRom(
    const Vector3& Position0, const Vector3& Position1, const Vector3& Position2,
    const Vector3& Position3, float t);
// OBBの頂点
void OBBIndex(const OBB& obb, std::vector<Vector3>& output_vertices);
// 分離軸
bool SeparationAxis(const Vector3 axis, const OBB obb_1, const OBB obb_2);