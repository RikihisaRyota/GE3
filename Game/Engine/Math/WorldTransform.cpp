#include "WorldTransform.h"
#include <cassert>
#include <d3dx12.h>
#include "../DirectXCommon/DirectXCommon.h"
#include "MyMath.h"

void WorldTransform::Initialize()
{
	CreateConstBuffer();
	Map();
	Reset();
	UpdateMatrix();
}

void WorldTransform::CreateConstBuffer()
{
	HRESULT result = S_FALSE;
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc =
		CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataWorldTransform) + 0xff) & ~0xff);

	// 定数バッファの生成
	result = DirectXCommon::GetInstance()->GetDevice()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&constBuff_));
	assert(SUCCEEDED(result));
}

void WorldTransform::Map()
{
	// 定数バッファとのデータリンク
	HRESULT result = constBuff_->Map(0, nullptr, (void**)&constMap_);
	assert(SUCCEEDED(result));
}

void WorldTransform::TransferMatrix()
{
	// 定数バッファに書き込み
	constMap_->matWorld = matWorld_;
}

void WorldTransform::UpdateMatrix()
{
	Matrix4x4 matScale, matRot, matTrans;

	// スケール、回転、平行移動行列の計算
	matScale = MakeScaleMatrix(scale_);
	matRot = MakeIdentity4x4();
	matRot *= MakeRotateZMatrix(rotation_.z);
	matRot *= MakeRotateXMatrix(rotation_.x);
	matRot *= MakeRotateYMatrix(rotation_.y);
	matTrans = MakeTranslateMatrix(translation_);

	// ワールド行列の合成
	matWorld_ = MakeIdentity4x4(); // 変形をリセット
	matWorld_ *= matScale;          // ワールド行列にスケーリングを反映
	matWorld_ *= matRot;            // ワールド行列に回転を反映
	matWorld_ *= matTrans;          // ワールド行列に平行移動を反映

	// もし親があれば
	if (parent_) {
		matWorld_ = Mul(matWorld_, parent_->matWorld_);
	}
	// 定数バッファに転送する
	TransferMatrix();
}

void WorldTransform::Reset() {
	// scale
	scale_ = { 1.0f,1.0f,1.0f };
	// rotatition
	rotation_ = { 0.0f,0.0f,0.0f };
	// translation
	translation_ = { 0.0f,0.0f,0.0f };
	// matWorld
	matWorld_ = MakeIdentity4x4();
}
