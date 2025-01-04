#pragma once
/**
 * @file BackGround.h
 * @brief 背景オブジェクト
 */
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"

#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"

class BackGround {
public:
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw();
	// デバック
	void DrawImGui();
	// Setter
	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }

private:
	GPUParticleManager* gpuParticleManager_;
	ModelHandle modelHandle_;

	GPUParticleShaderStructs::VertexEmitterForCPU vertexEmitter_;
	GPUParticleShaderStructs::MeshEmitterForCPU meshEmitter_;

	WorldTransform worldTransform_;
};