#pragma once

#include <memory>

#include "../Graphics/UploadBuffer.h"

#include "Matrix4x4.h"
#include "Vector3.h"


struct ConstBufferDataViewProjection {
	Matrix4x4 view; // ワールド->ビューに変換
	Matrix4x4 projection; // ビュー->プロジェクション行列
	Vector3 cameraPos;    // カメラ座標（ワールド座標）
};
struct ViewProjection
{
	// 定数バッファ
	UploadBuffer constBuff_;
	// マッピング済みアドレス
	ConstBufferDataViewProjection* constMap_ = nullptr;
#pragma region ビュー行列の設定
	// X,Y,Z軸回りのローカル回転角
	Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };
	// ローカル座標
	Vector3 translation_ = { 0.0f, 0.0f, -10.0f };
#pragma endregion ビュー行列の設定

#pragma region 射影行列の設定
	// 垂直方向視野角
	float fovAngleY_ = 45.0f * 3.141592654f / 180.0f;
	// ビューポートのアスペクト比
	float aspectRatio_ = (float)16 / 9;
	// 深度限界（手前側）
	float nearZ_ = 0.1f;
	// 深度限界（奥側）
	float farZ_ = 100.0f;
#pragma endregion

	// ビュー行列
	Matrix4x4 matView_;
	// 射影行列
	Matrix4x4 matProjection_;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 定数バッファ生成
	/// </summary>
	void CreateConstBuffer();
	/// <summary>
	/// マッピングする
	/// </summary>
	void Map();
	/// <summary>
	/// 行列を更新する
	/// </summary>
	void UpdateMatrix();
	/// <summary>
	/// 行列を転送する
	/// </summary>
	void TransferMatrix();

	void SetViewProjection(const ViewProjection* viewProjection);
};