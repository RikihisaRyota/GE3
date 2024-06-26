#pragma once

#include <memory>
#include <vector>

#include "Engine/Animation/Animation.h"
#include "Engine/Animation/Skinning.h"

#include "Engine/Graphics/CommandSignature.h"
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
class ModelHandle;
struct ViewProjection;
struct WorldTransform;
class GPUParticleManager {
public:
	void Initialize();

	void Update(const ViewProjection& viewProjection,CommandContext& commandContext);

	void Draw(const ViewProjection& viewProjection,CommandContext& commandContext);

	void CreateMeshParticle(const ModelHandle& modelHandle, Animation::Animation& animation , const WorldTransform& worldTransform ,CommandContext& commandContext);
	void CreateMeshParticle(const ModelHandle& modelHandle,const WorldTransform& worldTransform ,CommandContext& commandContext);
	void SetEmitter(const GPUParticleShaderStructs::EmitterForCPU& emitterForCPU);
	void SetBullets(const std::vector<GPUParticleShaderStructs::BulletForGPU>& bullets);

private:
	void CreateParticleBuffer();
	void CreateGraphics();
	void CreateEmitter();
	void CreateAddEmitter();
	void CreateUpdate();
	void CreateSpawn();
	void CreateIndexBuffer();
	void CreateBullet();
	void CreateMeshParticle();

	std::unique_ptr<PipelineState> graphicsPipelineState_;
	std::unique_ptr<RootSignature> graphicsRootSignature_;
	std::unique_ptr<RootSignature> spawnComputeRootSignature_;
	std::unique_ptr<PipelineState> spawnComputePipelineState_;
	std::unique_ptr<RootSignature> emitterUpdateComputeRootSignature_;
	std::unique_ptr<PipelineState> emitterUpdateComputePipelineState_;
	std::unique_ptr<RootSignature> checkEmitterComputeRootSignature_;
	std::unique_ptr<PipelineState> checkEmitterComputePipelineState_;
	std::unique_ptr<RootSignature> addEmitterComputeRootSignature_;
	std::unique_ptr<PipelineState> addEmitterComputePipelineState_;
	std::unique_ptr<RootSignature> updateComputeRootSignature_;
	std::unique_ptr<PipelineState> updateComputePipelineState_;
	std::unique_ptr<CommandSignature> commandSignature_;
	std::unique_ptr<CommandSignature> spawnCommandSignature_;
	std::unique_ptr<PipelineState> bulletPipelineState_;
	std::unique_ptr<RootSignature> bulletRootSignature_;
	std::unique_ptr<PipelineState> meshParticlePipelineState_;
	std::unique_ptr<RootSignature> meshParticleRootSignature_;

	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};

	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	// ランダムバッファ
	UploadBuffer randomBuffer_;

	std::unique_ptr<GPUParticle> gpuParticle_;
};