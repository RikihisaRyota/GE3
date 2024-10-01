#pragma once

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

	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }

private:
	GPUParticleManager* gpuParticleManager_;

	WorldTransform worldTransform_;
};