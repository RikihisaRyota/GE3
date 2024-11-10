#pragma once

#include <array>
#include <memory>
#include <vector>
#include <cstdint>
#include <string>

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
	void CollisionField(CommandContext& commandContext,const UploadBuffer& random);
	void Spawn(CommandContext& commandContext, const UploadBuffer& random);
	void UpdateEmitter(CommandContext& commandContext);
	void CheckEmitter(CommandContext& commandContext);
	void AddEmitter(CommandContext& commandContext);
	void ParticleUpdate(const ViewProjection& viewProjection, CommandContext& commandContext);
	void UpdateTrails(const ViewProjection& viewProjection, CommandContext& commandContext);
	void AddTrailsVertex(CommandContext& commandContext);
	void BulletUpdate(CommandContext& commandContext, const UploadBuffer& random);
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawTrails(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawImGui();
	void CreateMeshParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateMeshParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateVertexParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateVertexParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateEdgeParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext);
	void CreateEdgeParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext);

	//void CreateTransformModelParticle(const ModelHandle& startModelHandle, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformModelEmitterForGPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);
	//void CreateTransformModelParticle(const ModelHandle& startModelHandle, Animation::Animation& startAnimation, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, Animation::Animation& endAnimation, const Matrix4x4& endWorldTransform,  const GPUParticleShaderStructs::TransformModelEmitterForGPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);
	//void CreateTransformModelParticle(const ModelHandle& startModelHandle, Animation::Animation& startAnimation, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformModelEmitterForGPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);
	//void CreateTransformModelParticle(const ModelHandle& startModelHandle,  const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, Animation::Animation& endAnimation, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformModelEmitterForGPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);
	//
	//void CreateTransformModelAreaParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform,  const GPUParticleShaderStructs::TransformAreaEmitterForGPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext);


	void SetDrawCommandSignature(CommandSignature* commandSignature) { commandSignature_ = commandSignature; }
	void SetSpawnCommandSignature(CommandSignature* commandSignature) { spawnCommandSignature_ = commandSignature; }
	void SetTrailsDrawCommandSignature(CommandSignature* commandSignature) { trailsDrawCommandSignature_ = commandSignature; }

	void SetField(const GPUParticleShaderStructs::FieldForCPU& fieldForCPU);
	void SetEmitter(const GPUParticleShaderStructs::EmitterForCPU& emitterForCPU, const Matrix4x4& parent);
	void SetEmitter(const GPUParticleShaderStructs::VertexEmitterForCPU& emitterForCPU, const Matrix4x4& parent);
	void SetEmitter(const GPUParticleShaderStructs::MeshEmitterForCPU& emitterForCPU, const Matrix4x4& parent);
	void SetEmitter(const GPUParticleShaderStructs::TransformModelEmitterForCPU& emitterForCPU, const Matrix4x4& parent);
	void SetEmitter(const GPUParticleShaderStructs::TransformAreaEmitterForCPU& emitterForCPU, const Matrix4x4& parent);
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
	void InitializeTrails();

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
	// 描画引数用
	DefaultBuffer drawArgumentBuffer_;
	
	// エミッターのIndexと何個生成するか
	DefaultBuffer createParticleBuffer_;
	UploadBuffer resetCounterBuffer_;
	//DescriptorHandle createParticleUAVHandle_;
	DefaultBuffer spawnArgumentBuffer_;
	DefaultBuffer originalCounterBuffer_;
	CommandSignature* spawnCommandSignature_;
	// 何個生成するか数える用
	DefaultBuffer createParticleCounterCopySrcBuffer_;
	// 軌跡用
	CommandSignature* trailsDrawCommandSignature_;
	DefaultBuffer trailsArgumentBuffers_;
	DefaultBuffer trailsStockBuffers_;
	DefaultBuffer trailsIndexBuffers_;
	DefaultBuffer trailsDataBuffers_;
	DefaultBuffer trailsHeadBuffers_;
	DefaultBuffer trailsPositionBuffers_;
	DefaultBuffer trailsVertexDataBuffers_;
	DefaultBuffer trailsIndiesBuffers_;
	DefaultBuffer trailsDrawInstanceCountBuffers_;

	struct EmitterDesc {
		void Initialize(CommandContext& commandContext);
		void Create(const std::wstring& name, const GPUParticleShaderStructs::EmitterType& type);
		size_t CheckEmitter(CommandContext& commandContext, size_t emitterCount,void* data);
		void AddEmitter(CommandContext& commandContext);
		void UpdateEmitter(CommandContext& commandContext);
		void Spawn(CommandContext& commandContext);
		void UpdateParticle(CommandContext& commandContext);
		DefaultBuffer originalBuffer;
		//UploadBuffer uploadCopyBuffer;
		DefaultBuffer defaultCopyBuffer;
		DefaultBuffer  addCountBuffer;
		DefaultBuffer createEmitterBuffer;
		GPUParticleShaderStructs::EmitterType type;
	};
	EmitterDesc emitterForGPUDesc_;
	std::vector<GPUParticleShaderStructs::EmitterForGPU> emitterForGPUs_;

	EmitterDesc vertexEmitterForGPUDesc_;
	std::vector<GPUParticleShaderStructs::VertexEmitterForGPU> vertexEmitterForGPUs_;
	
	EmitterDesc meshEmitterForGPUDesc_;
	std::vector<GPUParticleShaderStructs::MeshEmitterForGPU> meshEmitterForGPUs_;
	
	EmitterDesc transformModelEmitterForGPUDesc_;
	std::vector<GPUParticleShaderStructs::TransformModelEmitterForGPU> transformModelEmitterForGPUs_;
	
	EmitterDesc transformAreaEmitterForGPUDesc_;
	std::vector<GPUParticleShaderStructs::TransformAreaEmitterForGPU> transformAreaEmitterForGPUs_;
	//// AddParticle用
	//UploadBuffer emitterCopyUploadBuffer_;
	//DefaultBuffer emitterCopyDefaultBuffer_;
	//// 追加するエミッターが何個あるか
	//UploadBuffer addEmitterCountBuffer_;
	//std::vector<GPUParticleShaderStructs::EmitterForGPU> emitterForGPUs_;
	//DefaultBuffer createEmitterBuffer_;

	//UploadBuffer meshEmitterCopyUploadBuffer_;
	//DefaultBuffer meshEmitterCopyDefaultBuffer_;
	//UploadBuffer addMeshEmitterCountBuffer_;
	//std::vector<GPUParticleShaderStructs::MeshEmitterForGPU> meshEmitterForGPUs_;
	//DefaultBuffer createMeshEmitterBuffer_;


	// 弾
	StructuredBuffer bulletsBuffer_;
	UploadBuffer bulletCountBuffer_;
	std::vector<GPUParticleShaderStructs::BulletForGPU> bullets_;
	// フィールド
	DefaultBuffer fieldOriginalBuffer_;
	UploadBuffer fieldCPUBuffer_;
	DefaultBuffer fieldAddBuffer_;
	UploadBuffer fieldCounterBuffer_;
	DefaultBuffer fieldIndexStockBuffer_;
	DefaultBuffer fieldIndexBuffer_;
	DefaultBuffer createFieldNumBuffer_;
	std::vector<GPUParticleShaderStructs::FieldForGPU> fields_;

	TextureHandle texture_;

	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}
};