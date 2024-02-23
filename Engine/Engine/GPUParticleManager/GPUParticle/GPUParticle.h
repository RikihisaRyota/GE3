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
	void EmitterUpdate(CommandContext& commandContext);
	void AddEmitter(CommandContext& commandContext);
	void ParticleUpdate(CommandContext& commandContext);
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void SetCommandSignature(ID3D12CommandSignature* commandSignature) { commandSignature_ = commandSignature; }
	void Create(const EmitterForGPU& emitterForGPU, TextureHandle textureHandle);

	void SetEmitter(const EmitterForGPU& emitterForGPU);
private:
	static const UINT ComputeThreadBlockSize = 1024;
	static const UINT MaxParticleNum = 1 << 25;
	// hlsli側も変更するように
	static const UINT MaxEmitterNum = 100;

	void InitializeParticleBuffer();
	void InitializeUpdateParticle();
	void InitializeBuffer();
	void InitializeEmitter();
	void InitializeAddEmitter();

	// コマンドシグネイチャ
	ID3D12CommandSignature* commandSignature_;
	// パーティクルの情報
	GpuResource particleBuffer_;
	// パーティクルのIndexをAppend,Consumeするよう
	GpuResource originalCommandBuffer_;
	DescriptorHandle originalCommandUAVHandle_;
	// パーティクルが何体生きているかをCPU側に伝えるコピー用
	GpuResource originalCommandCounterBuffer_;
	uint32_t* originalCommandCounter_;
	void* originalCommandCounterDate_;
	// 何番目のパーティクルが生きているか積み込みよう(ExecuteIndirect用)
	GpuResource drawIndexCommandBuffers_;
	DescriptorHandle drawIndexCommandUAVHandle_;
	UploadBuffer resetAppendDrawIndexBufferCounterReset_;
	// 描画引数用
	GpuResource drawArgumentBuffer_;
	DescriptorHandle drawArgumentHandle_;
	// パーティクルのエミッター
	GpuResource emitterForGPUBuffer_;
	// エミッターのIndexと何個生成するか
	GpuResource createParticleBuffer_;
	DescriptorHandle createParticleUAVHandle_;
	// 何個生成するか数える用
	GpuResource createParticleCounterCopyDestBuffer_;
	GpuResource createParticleCounterCopySrcBuffer_;
	uint32_t* createParticleCounter_;
	void* createParticleCounterDate_;
	// AddParticle用
	GpuResource addParticleBuffer_;
	UploadBuffer addParticleCopyBuffer_;
	// 追加するエミッターが何個あるか
	UploadBuffer addParticleCountBuffer_;
	DescriptorHandle addParticleUAVHandle_;
	std::vector<EmitterForGPU> emitterForGPUs_;


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