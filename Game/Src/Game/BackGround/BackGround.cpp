#include "BackGround.h"
#include "Engine//Model/ModelManager.h"

void BackGround::Initialize() {
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/BackGround/backGround.gltf");
	GPUParticleShaderStructs::Load("backGround", vertexEmitter_);
	GPUParticleShaderStructs::Load("backGround", meshEmitter_);
}

void BackGround::Update() {
	gpuParticleManager_->SetVertexEmitter(modelHandle_,vertexEmitter_);
	gpuParticleManager_->SetMeshEmitter(modelHandle_, meshEmitter_);


}

void BackGround::Draw() {

}

void BackGround::DrawImGui() {
#ifdef _DEBUG
	GPUParticleShaderStructs::Debug("backGround", vertexEmitter_);
	GPUParticleShaderStructs::Debug("backGround", meshEmitter_);
#endif // _DEBUG
}
