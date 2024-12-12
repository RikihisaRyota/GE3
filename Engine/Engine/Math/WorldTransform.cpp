#include "WorldTransform.h"

#include <cassert>
#include <d3dx12.h>

#include "MyMath.h"
#include "Engine/Model/ModelManager.h"

void WorldTransform::Initialize() {
	Reset();
	UpdateMatrix();
}

//void WorldTransform::TransferMatrix(const ModelHandle& modelHandle, uint32_t children) {
//	// 定数バッファに書き込み
//	if (modelHandle != ModelHandle::kMaxModeHandle) {
//		// マルチメッシュに対応してない
//		constMap->matWorld = ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(children)->rootNode.localMatrix * matWorld;
//	}
//	else {
//		constMap->matWorld = matWorld;
//	}
//	constMap->inverseMatWorld = Transpose(Inverse(matWorld));
//	Map();
//}


void WorldTransform::UpdateMatrix() {
	Matrix4x4 matScale, matRot, matTrans;

	// スケール、回転、平行移動行列の計算
	matScale = MakeScaleMatrix(scale);
	matRot = MakeIdentity4x4();
	matRot *= MakeRotate(rotate);
	matTrans = MakeTranslateMatrix(translate);

	// ワールド行列の合成
	matWorld = MakeIdentity4x4(); // 変形をリセット
	matWorld *= matScale;          // ワールド行列にスケーリングを反映
	matWorld *= matRot;            // ワールド行列に回転を反映
	matWorld *= matTrans;          // ワールド行列に平行移動を反映


	// もし親があれば
	if (parent_) {
		matWorld = Mul(matWorld, parent_->matWorld);
	}
}

void WorldTransform::Reset() {
	// scale
	scale = { 1.0f,1.0f,1.0f };
	// rotatition
	rotate = Quaternion::identity;
	// translation
	translate = { 0.0f,0.0f,0.0f };
	// matWorld
	matWorld = MakeIdentity4x4();
}