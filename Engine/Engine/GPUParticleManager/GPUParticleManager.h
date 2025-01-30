#pragma once
/**
 * @file GPUParticleManager.h
 * @brief GPUParticleManager
 */
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

#include "Engine/Math/MyMath.h"
class CommandContext;
class ModelHandle;
struct ViewProjection;
struct WorldTransform;
class GPUParticleManager {
public:
	void Initialize();

	void Update(const ViewProjection& viewProjection, CommandContext& commandContext);

	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	// デバック用
	void DrawImGui();

	// セッター
	void SetEmitter(const GPUParticleShaderStructs::EmitterForCPU& emitter,const Matrix4x4& parent=Matrix4x4());
	void SetVertexEmitter(const ModelHandle& modelHandle, const GPUParticleShaderStructs::VertexEmitterForCPU& emitter,const Matrix4x4& parent=Matrix4x4());
	void SetVertexEmitter(const ModelHandle& modelHandle, const Engine::Animation::Animation& animation, const GPUParticleShaderStructs::VertexEmitterForCPU& emitter,const Matrix4x4& parent=MakeIdentity4x4());
	void SetMeshEmitter(const ModelHandle& modelHandle, const GPUParticleShaderStructs::MeshEmitterForCPU& emitter,const Matrix4x4& parent=Matrix4x4());
	void SetMeshEmitter(const ModelHandle& modelHandle, const Engine::Animation::Animation& animation, const GPUParticleShaderStructs::MeshEmitterForCPU& emitter,const Matrix4x4& parent= MakeIdentity4x4());
	void SetTransformModelEmitter(const ModelHandle& startModelHandle,const ModelHandle& endModelHandle, const GPUParticleShaderStructs::TransformModelEmitterForCPU& emitter,const Matrix4x4& parent= MakeIdentity4x4());
	void SetTransformAreaEmitter(const ModelHandle& modelHandle, const GPUParticleShaderStructs::TransformAreaEmitterForCPU& emitter,const Matrix4x4& parent= MakeIdentity4x4());
	void SetField(const GPUParticleShaderStructs::FieldForCPU& fieldForCPU);
	void SetBullet(const GPUParticleShaderStructs::BulletForGPU& bullets);
private:
	// 初期化用関数
	void CreateParticleBuffer();
	void CreateGraphics();
	void CreateEmitter();
	void CreateUpdate();
	void CreateUpdateTrails();
	void CreateIndexBuffer();
	void CreateBullet();
	void CreateMeshParticle();
	void CreateField();
	void CreateTranslateModelParticle();


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

	std::unique_ptr<RootSignature> checkFieldRootSignature_;
	std::unique_ptr<PipelineState> checkFieldPipelineState_;
	std::unique_ptr<RootSignature> addFieldRootSignature_;
	std::unique_ptr<PipelineState> addFieldPipelineState_;
	std::unique_ptr<RootSignature> updateFieldRootSignature_;
	std::unique_ptr<PipelineState> updateFieldPipelineState_;
	std::unique_ptr<RootSignature> collisionFieldRootSignature_;
	std::unique_ptr<PipelineState> collisionFieldPipelineState_;

	std::unique_ptr<CommandSignature> trailsDrawCommandSignature_;
	std::unique_ptr<RootSignature> updateTrailsRootSignature_;
	std::unique_ptr<PipelineState> updateTrailsPipelineState_;

	std::unique_ptr<RootSignature> addVertexTrailsRootSignature_;
	std::unique_ptr<PipelineState> addVertexTrailsPipelineState_;

	//std::unique_ptr<RootSignature> translateModelParticleRootSignature_;
	//std::unique_ptr<PipelineState> translateModelParticlePipelineState_;
	//std::unique_ptr<RootSignature> translateModelAreaParticleRootSignature_;
	//std::unique_ptr<PipelineState> translateModelAreaParticlePipelineState_;

	std::unique_ptr<RootSignature> tailsRootSignature_;
	std::unique_ptr<PipelineState> tailsPipelineState_;
	std::unique_ptr<PipelineState> tailsGraphicsPipelineState_;
	std::unique_ptr<RootSignature> tailsGraphicsRootSignature_;
	UploadBuffer vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vbView_{};
	
	UploadBuffer indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW ibView_{};

	// ランダムバッファ
	UploadBuffer randomBuffer_;

	std::unique_ptr<GPUParticle> gpuParticle_;
};