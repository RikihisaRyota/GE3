#pragma once

#include <memory>
#include <vector>

#include <wrl.h>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/GpuResource.h"
#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"

class GPUParticle {
private:
	struct Particle {
		Vector3 position;
		Vector3 velocity;
	};
	struct ParticleInfo {
		float speed;
	};
public:
	GPUParticle();
	void Initialize();
	void Update();
	void Render(CommandContext& commandContext);

private:
	void InitializeSpawnParticle();
	void InitializeUpdateParticle();

	std::unique_ptr<PipelineState> graphicsPipelineState_;
	std::unique_ptr<RootSignature> graphicsRootSignature_;
	std::unique_ptr<RootSignature> spawnComputeRootSignature_;
	std::unique_ptr<PipelineState> spawnComputePipelineState_;
	std::unique_ptr<RootSignature> updateComputeRootSignature_;
	std::unique_ptr<PipelineState> updateComputePipelineState_;

	GpuResource rwStructuredBuffer_;
	// UAVハンドル
	DescriptorHandle uavHandle_;
	
	// Update用
	ParticleInfo* particleInfo_;
	Particle* particle_;
	UploadBuffer updateConstantBuffer_;
	static const uint32_t kNumThread = 256;
};