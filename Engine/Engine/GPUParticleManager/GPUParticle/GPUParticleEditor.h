#pragma once

#include <memory>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/DefaultBuffer.h"
#include "Engine/Graphics/PipelineState.h"
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
	void CreateGraphics();
	void CreateSpawn();
	void CreateIndexBuffer();
	void CreateEmitterBuffer();

	void CreateParticleBuffer();

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

	UploadBuffer emitterBuffer_;

	Emitter emitter_;

	DefaultBuffer particleBuffer_;
};