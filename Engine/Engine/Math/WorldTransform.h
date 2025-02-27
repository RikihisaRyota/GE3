#pragma once
/**
 * @file WorldTransform.h
 */

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
	// マッピング済みアドレス
	//ConstBufferDataWorldTransform* constMap = nullptr;
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
	/// 行列を更新する
	/// </summary>
	void UpdateMatrix();

	/// <summary>
	/// メンバ変数の初期化
	/// </summary>
	void Reset();
};