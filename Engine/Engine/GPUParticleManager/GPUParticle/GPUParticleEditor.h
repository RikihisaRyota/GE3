#pragma once

#include <memory>

#include "Engine/Graphics/DescriptorHandle.h"
#include "Engine/Graphics/DefaultBuffer.h"
#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/ReadBackBuffer.h"
#include "Engine/Graphics/RootSignature.h"
#include "GPUParticleShaderStructs.h"

class CommandContext;
struct ViewProjection;
class GPUParticleEditor {
public:
	void Initialize();

	void Update(CommandContext& commandContext);

	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	void CreateParticle(const Emitter& emitterForGPU);
private:
	void Spawn(CommandContext& commandContext);
	void ParticleUpdate(CommandContext& commandContext);

	void CreateGraphics();
	void CreateSpawn();
	void CreateIndexBuffer();
	void CreateEmitterBuffer();

	void CreateParticleBuffer();
	void CreateUpdateParticle();
	void CreateBuffer();

	std::unique_ptr<PipelineState> graphicsPipelineState_;
	std::unique_ptr<RootSignature> graphicsRootSignature_;
	std::unique_ptr<RootSignature> emitterUpdateComputeRootSignature_;
	std::unique_ptr<PipelineState> emitterUpdateComputePipelineState_;
	std::unique_ptr<RootSignature> spawnComputeRootSignature_;
	std::unique_ptr<PipelineState> spawnComputePipelineState_;
	std::unique_ptr<RootSignature> updateComputeRootSignature_;
	std::unique_ptr<PipelineState> updateComputePipelineState_;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSignature_;

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	UINT particleIndexSize_;
	UINT particleIndexCounterOffset_;

	UploadBuffer emitterBuffer_;

	Emitter emitter_;

	DefaultBuffer particleBuffer_;

	// 何番目のパーティクルが生きているか積み込みよう(ExecuteIndirect用)
	DefaultBuffer drawIndexCommandBuffers_;
	DescriptorHandle drawIndexCommandUAVHandle_;
	UploadBuffer resetAppendDrawIndexBufferCounterReset_;
	// パーティクルが何体生きているかをCPU側に伝えるコピー用
	ReadBackBuffer originalCommandCounterBuffer_;
	// パーティクルのIndexをAppend,Consumeするよう
	DefaultBuffer originalCommandBuffer_;
	DescriptorHandle originalCommandUAVHandle_;
	// 描画引数用
	DefaultBuffer drawArgumentBuffer_;
	DescriptorHandle drawArgumentHandle_;

	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}
};