#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include <wrl.h>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/DescriptorHandle.h"

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Matrix4x4.h"

struct ViewProjection;
class GPUParticle {
private:
	struct Particle {
		Vector3 velocity;
		float scale;
		Vector3 rotate;
		Vector3 translate;
	};
	struct ParticleInfo {
		float speed;
	};
public:
	GPUParticle();
	void Initialize();
	void Update();
	void Render(const ViewProjection& viewProjection);

private:
	void InitializeSpawnParticle();
	void InitializeUpdateParticle();
	void InitializeGraphics();

	std::unique_ptr<PipelineState> graphicsPipelineState_;
	std::unique_ptr<RootSignature> graphicsRootSignature_;
	std::unique_ptr<RootSignature> spawnComputeRootSignature_;
	std::unique_ptr<PipelineState> spawnComputePipelineState_;
	std::unique_ptr<RootSignature> updateComputeRootSignature_;
	std::unique_ptr<PipelineState> updateComputePipelineState_;

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};
	std::vector<uint16_t> indices_;

	GpuResource rwStructuredBuffer_;
	// UAVハンドル
	DescriptorHandle uavHandle_;

	DescriptorHandle rwStructuredBufferHandle_;

	ParticleInfo* particleInfo_;
	Particle* particle_;
	UploadBuffer updateConstantBuffer_;
	static const uint32_t kNumThread = 100000;
};