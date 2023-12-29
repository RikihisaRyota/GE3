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
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Matrix4x4.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Model/ModelHandle.h"

struct ViewProjection;
class GPUParticle {
private:
	struct Vertex {
		Vector3 position;
	};
	struct Particle {
		float scale;
		Vector3 velocity;
		Vector3 rotate;
		float pad;
		Vector3 translate;
	};
	struct ParticleInfo {
		float speed;
	};
	struct IndirectCommand {
		D3D12_GPU_VIRTUAL_ADDRESS cbv;
		D3D12_DRAW_ARGUMENTS drawArguments;
	};

public:
	GPUParticle();
	void Initialize();
	void Update();
	void Render(const ViewProjection& viewProjection);

private:
	static const UINT kNumThread;
	static const UINT CommandSizePerFrame;
	static const UINT CommandBufferCounterOffset;

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
	std::vector<Vertex>	vertices_;

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};
	std::vector<uint16_t> indices_;

	GpuResource rwStructuredBuffer_;
	DescriptorHandle rwStructuredBufferHandle_;
	// UAVハンドル
	DescriptorHandle uavHandle_;
	
	// モデル
	ModelHandle modelHandle_;
	WorldTransform worldTransform_;

	Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSignature_;
	GpuResource commandBuffer_;
	UploadBuffer commandUploadBuffer_;
	DescriptorHandle commandHandle_;
	// 計算結果格納用
	GpuResource processedCommandBuffers_;
	DescriptorHandle processedCommandsHandle_;

	ParticleInfo* particleInfo_;
	Particle* particle_;
	UploadBuffer updateConstantBuffer_;

	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}
};