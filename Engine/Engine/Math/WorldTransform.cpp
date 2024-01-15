#include "WorldTransform.h"
#include <cassert>
#include <d3dx12.h>
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
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(L"WorldTransform", sizeof(ConstBufferDataWorldTransform));
	constMap_ = new ConstBufferDataWorldTransform();
}

void WorldTransform::Map()
{
	// 定数バッファとのデータリンク
	constBuff_->Copy(constMap_, sizeof(ConstBufferDataWorldTransform));
}

void WorldTransform::TransferMatrix()
{
	// 定数バッファに書き込み
	constMap_->matWorld = matWorld_;
	Map();
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
