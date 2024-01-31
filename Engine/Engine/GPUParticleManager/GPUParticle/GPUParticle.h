#pragma once

#include <array>
#include <memory>
#include <vector>
#include <cstdint>

#include <wrl.h>

#include "Engine/Graphics/UploadBuffer.h"
#include "Engine/Graphics/PipelineState.h"
#include "Engine/Graphics/RootSignature.h"
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
class CommandContext;
struct ViewProjection;
class GPUParticle {
private:
	enum CommandSigunature {
		kParticleSRV,
		kDrawIndexSRV,
		kDrawIndex,

		kCommandSigunatureCount,
	};
public:
	GPUParticle();
	~GPUParticle();
	void Initialize();
	void Spawn(CommandContext& commandContext);
	void Update(CommandContext& commandContext);
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void SetCommandSignature(ID3D12CommandSignature* commandSignature) { commandSignature_ = commandSignature; }
	void Create(const EmitterForGPU& emitterForGPU, TextureHandle textureHandle);

	void SetEmitter(const EmitterForGPU& emitterForGPU);
	const EmitterForGPU& GetEmitter () { return emitterForGPU_; }
private:
	static const UINT ComputeThreadBlockSize = 1024;

	void InitializeParticleBuffer();
	void InitializeUpdateParticle();
	void InitializeEmitter(const EmitterForGPU& emitterForGPU);

	// コマンドシグネイチャ
	ID3D12CommandSignature* commandSignature_;
	// パーティクルの情報
	GpuResource particleBuffer_;
	DescriptorHandle rwStructuredBufferHandle_;
	// パーティクルの生きてるか判定用
	GpuResource originalCommandBuffer_;
	uint32_t* originalCommandCounter_;
	void* originalCommandCounterDate_;
	GpuResource originalCommandCounterBuffer_;
	DescriptorHandle originalCommandUAVHandle_;
	// drawIndexBuffer
	GpuResource drawIndexCommandBuffers_;
	DescriptorHandle drawIndexCommandUAVHandle_;
	UploadBuffer resetAppendDrawIndexBufferCounterReset_;
	// 描画引数用
	GpuResource drawArgumentBuffer_;
	DescriptorHandle drawArgumentHandle_;
	// パーティクルのエミッター
	EmitterForGPU emitterForGPU_;
	//GpuResource emitterForGPUBuffer_;
	UploadBuffer emitterForGPUBuffer_;
	DescriptorHandle emitterForGPUSRVHandle_;

	UINT commandSizePerFrame_;
	UINT drawIndexBufferCounterOffset_;

	TextureHandle texture_;

	float time_ = 0.0f;

	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}
};