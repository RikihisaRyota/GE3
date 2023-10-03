#include "ViewProjection.h"
#include "../WinApp/WinApp.h"
#include <cassert>
#include <d3dx12.h>
#include "../DirectXCommon/DirectXCommon.h"
#include "MyMath.h"

void ViewProjection::Initialize() {
	// 定数バッファの生成
	CreateConstBuffer();

	// マッピング
	Map();

	//　行列の生成
	UpdateMatrix();
}

void ViewProjection::CreateConstBuffer() {
	HRESULT result;

	// ヒーププロパティ
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	// リソース設定
	CD3DX12_RESOURCE_DESC resourceDesc =
		CD3DX12_RESOURCE_DESC::Buffer((sizeof(ConstBufferDataViewProjection) + 0xff) & ~0xff);

	// 定数バッファの生成
	result = DirectXCommon::GetInstance()->DirectXCommon::GetDevice()->CreateCommittedResource(
		&heapProps, // アップロード可能
		D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&constBuff_));
	assert(SUCCEEDED(result));
}

void ViewProjection::Map() {
	// 定数バッファとのデータリンク
	HRESULT result = constBuff_->Map(0, nullptr, (void**)&constMap_);
	assert(SUCCEEDED(result));
}

void ViewProjection::UpdateMatrix() {
	// ビュー行列の生成
	matView_ = MakeViewMatrix(rotation_,translation_);

	// 平行投影による射影行列の生成
	//matProjection_ = MakeOrthographicMatrix(
	//	0.0f, 0.0f,
	//	static_cast<float> (WinApp::kWindowWidth),
	//	static_cast<float> (WinApp::kWindowHeight),
	//	nearZ_, farZ_);
	// 透視投影による射影行列の生成
	matProjection_ = MakePerspectiveFovMatrix(fovAngleY_, aspectRatio_, nearZ_, farZ_);

	// 定数バッファに書き込み
	constMap_->view = matView_;
	constMap_->projection = matProjection_;
	constMap_->cameraPos = translation_;
}

void ViewProjection::TransferMatrix() {
	// 平行投影による射影行列の生成
	//matProjection_ = MakeOrthographicMatrix(
	//	0.0f, 0.0f,
	//	WinApp::kWindowWidth,
	//	WinApp::kWindowHeight,
	//	nearZ_, farZ_);
	// 透視投影による射影行列の生成
	matProjection_ = MakePerspectiveFovMatrix(fovAngleY_, aspectRatio_, nearZ_, farZ_);
	// 定数バッファに書き込み
	constMap_->view = matView_;
	constMap_->projection = matProjection_;
	constMap_->cameraPos = translation_;
}

void ViewProjection::SetViewProjection(const ViewProjection* viewProjection) {
	matView_ = viewProjection->matView_;
	matProjection_ = viewProjection->matProjection_;
	rotation_ = viewProjection->rotation_;
	translation_ = viewProjection->translation_;
}