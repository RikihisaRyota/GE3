#pragma once

#include <array>
#include <memory>
#include <vector>
#include <cstdint>

#include <wrl.h>

#include "Engine/Animation/Animation.h"
#include "Engine/Graphics/CommandSignature.h"
#include "Engine/Graphics/DefaultBuffer.h"
#include "Engine/Graphics/ReadBackBuffer.h"
#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
#include "Engine/Graphics/StructuredBuffer.h"
#include "Engine/Graphics/DescriptorHandle.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/Texture/TextureHandle.h"

#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Matrix4x4.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Model/ModelHandle.h"

class TextureHandle;
class ModelHandle;
class CommandContext;
struct ViewProjection;
struct WorldTransform;
class GPUParticle {
public:
	GPUParticle();
	~GPUParticle();
	void Initialize();
	void CheckField(CommandContext& commandContext);
	void AddField(CommandContext& commandContext);
	void UpdateField(CommandContext& commandContext);
	void CollisionField(CommandContext& commandContext);
	void Spawn(CommandContext& commandContext, const UploadBuffer& random);
	void EmitterUpdate(CommandContext& commandContext);
	void CheckEmitter(CommandContext& commandContext);
	void AddEmitter(CommandContext& commandContext);
	void ParticleUpdate(const ViewProjection& viewProjection, CommandContext& commandContext);
	void BulletUpdate(CommandContext& commandContext,  const UploadBuffer& random);
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawImGui();
	void CreateMeshParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateMeshParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateVertexParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateVertexParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateEdgeParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateEdgeParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext);

	void CreateTransformModelParticle(const ModelHandle& startModelHandle, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);
	void CreateTransformModelParticle(const ModelHandle& startModelHandle, Animation::Animation& startAnimation, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, Animation::Animation& endAnimation, const Matrix4x4& endWorldTransform,  const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);
	void CreateTransformModelParticle(const ModelHandle& startModelHandle, Animation::Animation& startAnimation, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);
	void CreateTransformModelParticle(const ModelHandle& startModelHandle,  const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, Animation::Animation& endAnimation, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);
	void CreateTransformModelAreaParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform,  const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);


	void SetDrawCommandSignature(CommandSignature* commandSignature) { commandSignature_ = commandSignature; }
	void SetSpawnCommandSignature(CommandSignature* commandSignature) { spawnCommandSignature_ = commandSignature; }

	void SetField(const GPUParticleShaderStructs::FieldForCPU& fieldForCPU);
	void SetEmitter(const GPUParticleShaderStructs::EmitterForCPU& emitterForGPU);
	void SetBullet(const GPUParticleShaderStructs::BulletForGPU& bullet);
private:
	void InitializeParticleBuffer();
	void InitializeUpdateParticle();
	void InitializeSpawnBuffer();
	void InitializeBuffer();
	void InitializeEmitter();
	void InitializeAddEmitter();
	void InitializeBullets();
	void InitializeField();

	std::unique_ptr<RootSignature> initializeBufferRootSignature_;
	std::unique_ptr<PipelineState> initializeBufferPipelineState_;

	// コマンドシグネイチャ
	CommandSignature* commandSignature_;
	// パーティクルの情報
	DefaultBuffer particleBuffer_;
	// パーティクルのIndexをAppend,Consumeするよう
	DefaultBuffer originalCommandBuffer_;
	// パーティクルが何体生きているかをCPU側に伝えるコピー用
	//ReadBackBuffer originalCommandCounterBuffer_;
	// 何番目のパーティクルが生きているか積み込みよう(ExecuteIndirect用)
	DefaultBuffer drawIndexCommandBuffers_;
	ReadBackBuffer drawIndexCountBuffer_;
	UploadBuffer resetAppendDrawIndexBufferCounterReset_;
	// 描画引数用
	DefaultBuffer drawArgumentBuffer_;
	// パーティクルのエミッター
	DefaultBuffer emitterForGPUBuffer_;
	// エミッターのIndexと何個生成するか
	DefaultBuffer createParticleBuffer_;
	UploadBuffer resetCreateParticleBuffer_;
	//DescriptorHandle createParticleUAVHandle_;
	DefaultBuffer spawnArgumentBuffer_;
	DefaultBuffer originalCounterBuffer_;
	CommandSignature* spawnCommandSignature_;
	// 何個生成するか数える用
	DefaultBuffer createParticleCounterCopySrcBuffer_;
	// AddParticle用
	UploadBuffer emitterCopyUploadBuffer_;
	DefaultBuffer emitterCopyDefaultBuffer_;
	// 追加するエミッターが何個あるか
	UploadBuffer addEmitterCountBuffer_;
	std::vector<GPUParticleShaderStructs::EmitterForGPU> emitterForGPUs_;
	DefaultBuffer createEmitterBuffer_;
	// 弾
	StructuredBuffer bulletsBuffer_;
	UploadBuffer bulletCountBuffer_;
	std::vector<GPUParticleShaderStructs::BulletForGPU> bullets_;
	// フィールド
	DefaultBuffer fieldOriginalBuffer_;
	//DescriptorHandle fieldOriginalHandle_;
	UploadBuffer fieldCPUBuffer_;
	DefaultBuffer fieldAddBuffer_;
	UploadBuffer fieldCounterBuffer_;
	DefaultBuffer fieldIndexStockBuffer_;
	DefaultBuffer fieldIndexBuffer_;
	DefaultBuffer createFieldNumBuffer_;
	std::vector<GPUParticleShaderStructs::FieldForGPU> fields_;

	// メッシュパーティクル
	UINT particleIndexSize_;
	UINT particleIndexCounterOffset_;

	UINT emitterIndexSize_;
	UINT emitterIndexCounterOffset_;

	UINT addEmitterSize_;
	UINT addEmitterCounterOffset_;

	TextureHandle texture_;

	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}
};