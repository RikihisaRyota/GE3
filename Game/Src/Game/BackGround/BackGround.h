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
	void Initialize();

	void Update();

	void Draw();

	void DrawImGui();

	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }

private:
	GPUParticleManager* gpuParticleManager_;
	ModelHandle modelHandle_;

	GPUParticleShaderStructs::VertexEmitterForCPU vertexEmitter_;
	GPUParticleShaderStructs::MeshEmitterForCPU meshEmitter_;

	WorldTransform worldTransform_;
};