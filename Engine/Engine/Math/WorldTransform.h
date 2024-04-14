#pragma once
#include <memory>

#include "Matrix4x4.h"
#include "Vector3.h"
#include "Quaternion.h"

#include "../Graphics/UploadBuffer.h"
#include "Engine/Model/ModelHandle.h"

// 定数バッファ用データ構造体
struct ConstBufferDataWorldTransform {
	Matrix4x4 matWorld; // ローカル → ワールド変換行列
	Matrix4x4 inverseMatWorld;
};

struct WorldTransform {
	// 定数バッファ
	std::unique_ptr<UploadBuffer> constBuff;
	// マッピング済みアドレス
	ConstBufferDataWorldTransform* constMap = nullptr;
	// scale
	Vector3 scale = { 1.0f,1.0f,1.0f };
	// rotation
	Quaternion rotate = { 0.0f,0.0f,0.0f,1.0f };
	// translation
	Vector3 translate = { 0.0f,0.0f,0.0f };
	// matWorld
	Matrix4x4 matWorld/* = MakeIdentity4x4()*/;
	// 親となるワールド変換へのポインタ
	const WorldTransform* parent_ = nullptr;
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
	/// 行列を転送する
	/// </summary>
	void TransferMatrix(const ModelHandle& modelHandle = ModelHandle::kMaxModeHandle, uint32_t children = 0);
	/// <summary>
	/// 行列を更新する
	/// </summary>
	void UpdateMatrix(const ModelHandle& modelHandle = ModelHandle::kMaxModeHandle, uint32_t children = 0);

	/// <summary>
	/// メンバ変数の初期化
	/// </summary>
	void Reset();
};