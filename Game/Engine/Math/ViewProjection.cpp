#include "ViewProjection.h"
#include "../WinApp/WinApp.h"
#include <cassert>
#include <d3dx12.h>
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
	constBuff_ = std::make_unique<UploadBuffer>();
	constBuff_->Create(L"ViewProjection", sizeof(ConstBufferDataViewProjection));
}

void ViewProjection::Map() {
	// 定数バッファとのデータリンク
	constBuff_->Copy(constMap_, sizeof(ConstBufferDataViewProjection));
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