#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"
#include <d3d12.h>
#include <wrl.h>

// 定数バッファ用データ構造体
struct ConstBufferDataWorldTransform {
	Matrix4x4 matWorld; // ローカル → ワールド変換行列
};

struct WorldTransform {
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> constBuff_;
	// マッピング済みアドレス
	ConstBufferDataWorldTransform* constMap_ = nullptr;
	// scale
	Vector3 scale_ = { 1.0f,1.0f,1.0f };
	// rotatition
	Vector3 rotation_ = { 0.0f,0.0f,0.0f };
	// translation
	Vector3 translation_ = { 0.0f,0.0f,0.0f };
	// matWorld
	Matrix4x4 matWorld_/* = MakeIdentity4x4()*/;
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
	void TransferMatrix();
	/// <summary>
	/// 行列を更新する
	/// </summary>
	void UpdateMatrix();
	
	/// <summary>
	/// メンバ変数の初期化
	/// </summary>
	void Reset();
};