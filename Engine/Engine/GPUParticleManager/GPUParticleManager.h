#pragma once

#include <memory>
#include <vector>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticle.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Matrix4x4.h"

class CommandContext;
struct ViewProjection;
class GPUParticleManager {
public:
	void Initialize();

	void Update(CommandContext& commandContext);

	void Draw(const ViewProjection& viewProjection,CommandContext& commandContext);

	void CreateParticle(const GPUParticleShaderStructs::Emitter& emitterForGPU);

private:
	void CreateParticleBuffer();
	void CreateGraphics();
	void CreateEmitter();
	void CreateAddEmitter();
	void CreateUpdate();
	void CreateSpawn();
	void CreateIndexBuffer();

	std::unique_ptr<PipelineState> graphicsPipelineState_;
	std::unique_ptr<RootSignature> graphicsRootSignature_;
	std::unique_ptr<RootSignature> spawnComputeRootSignature_;
	std::unique_ptr<PipelineState> spawnComputePipelineState_;
	std::unique_ptr<RootSignature> emitterUpdateComputeRootSignature_;
	std::unique_ptr<PipelineState> emitterUpdateComputePipelineState_;
	std::unique_ptr<RootSignature> addEmitterComputeRootSignature_;
	std::unique_ptr<PipelineState> addEmitterComputePipelineState_;
	std::unique_ptr<RootSignature> updateComputeRootSignature_;
	std::unique_ptr<PipelineState> updateComputePipelineState_;
	Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSignature_;

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	std::unique_ptr<GPUParticle> gpuParticle_;
};