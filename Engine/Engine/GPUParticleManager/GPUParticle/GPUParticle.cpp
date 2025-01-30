#include "GPUParticle.h"

#include <array>

#include <d3dx12.h>

#include "imgui.h"

#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/CommandListManager.h"
#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Input/Input.h"

GPUParticle::GPUParticle() {
	// resetするよう
	resetCounterBuffer_.Create(L"resetCounterBuffer", sizeof(UINT));
	resetCounterBuffer_.ResetBuffer();
	InitializeParticleBuffer();
	InitializeUpdateParticle();
	InitializeSpawnBuffer();
	InitializeEmitter();
	InitializeAddEmitter();
	InitializeBullets();
	InitializeField();
	InitializeTrails();
	InitializeBuffer();
}

GPUParticle::~GPUParticle() {}

void GPUParticle::Initialize() {}

void GPUParticle::CheckField(CommandContext& commandContext) {
	// エミッター追加
	if (!fields_.empty()) {
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"CheckField");
		size_t fieldCount = fields_.size();
		size_t copySize = sizeof(GPUParticleShaderStructs::FieldForCPU) * fieldCount;
		fieldCPUBuffer_.ResetBuffer();
		fieldCPUBuffer_.Copy(fields_.data(), copySize);
		// CPUの情報をコピー
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, fieldAddBuffer_, fieldCPUBuffer_);
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, createFieldNumBuffer_, sizeof(UINT), &fieldCount);

		commandContext.TransitionResource(QueueType::Type::COMPUTE, fieldAddBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, fieldBuffer_.buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, fieldAddBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 1, fieldBuffer_.buffer->GetGPUVirtualAddress());
		commandContext.Dispatch(QueueType::Type::COMPUTE, uint32_t(fieldCount), 1, 1);

		commandContext.UAVBarrier(QueueType::Type::COMPUTE, fieldBuffer_.buffer);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, fieldAddBuffer_);
		commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::AddField(CommandContext& commandContext) {
	// フィールド追加
	if (!fields_.empty()) {
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"AddField");

		fieldBuffer_.TransitionResource(QueueType::Type::COMPUTE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, commandContext);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, fieldAddBuffer_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, createFieldNumBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 0, fieldAddBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 1, fieldBuffer_.buffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, createFieldNumBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 3, fieldBuffer_.freeList.list->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 4, fieldBuffer_.freeList.index->GetGPUVirtualAddress());
		commandContext.Dispatch(QueueType::Type::COMPUTE, 1, 1, 1);

		fieldBuffer_.UavBarrier(QueueType::Type::COMPUTE, commandContext);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, createFieldNumBuffer_);

		commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::UpdateField(CommandContext& commandContext) {
	if (!fields_.empty()) {
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"UpdateField");
		// カウンターリセット
		commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, appendFieldIndexBuffer_, appendFieldIndexBuffer_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(UINT));

		fieldBuffer_.TransitionResource(QueueType::Type::COMPUTE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, commandContext);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, appendFieldIndexBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, fieldBuffer_.buffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 1, fieldBuffer_.freeList.list->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, fieldBuffer_.freeList.index->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 3, appendFieldIndexBuffer_.GetUAVHandle());
		commandContext.Dispatch(QueueType::Type::COMPUTE, 1, 1, 1);

		fieldBuffer_.UavBarrier(QueueType::Type::COMPUTE, commandContext);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, appendFieldIndexBuffer_);
		commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::CollisionField(CommandContext& commandContext, const UploadBuffer& random) {
	if (!fields_.empty()) {
		// 現在particleは二種類将来的には分ける
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"CollisionField");
		fieldBuffer_.TransitionResource(QueueType::Type::COMPUTE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, commandContext);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, appendFieldIndexBuffer_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 0, fieldBuffer_.buffer->GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 1, appendFieldIndexBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, directParticle_.buffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 3, computeParticle_.buffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 4, random.GetGPUVirtualAddress());
		commandContext.Dispatch(QueueType::Type::COMPUTE, static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), uint32_t(fields_.size()), 1);

		commandContext.UAVBarrier(QueueType::Type::COMPUTE, directParticle_.buffer);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, computeParticle_.buffer);
		fields_.clear();
		commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::CheckEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty() ||
		!vertexEmitterForGPUs_.empty() ||
		!meshEmitterForGPUs_.empty() ||
		!transformModelEmitterForGPUs_.empty() ||
		!transformAreaEmitterForGPUs_.empty()) {

		commandContext.BeginEvent(QueueType::Type::SPAWN, L"CheckEmitter");

		size_t sumEmitter = 0;
		sumEmitter += emitterForGPUDesc_.CheckEmitter(commandContext, emitterForGPUs_.size(), emitterForGPUs_.data());
		sumEmitter += vertexEmitterForGPUDesc_.CheckEmitter(commandContext, vertexEmitterForGPUs_.size(), vertexEmitterForGPUs_.data());
		sumEmitter += meshEmitterForGPUDesc_.CheckEmitter(commandContext, meshEmitterForGPUs_.size(), meshEmitterForGPUs_.data());
		sumEmitter += transformModelEmitterForGPUDesc_.CheckEmitter(commandContext, transformModelEmitterForGPUs_.size(), transformModelEmitterForGPUs_.data());
		sumEmitter += transformAreaEmitterForGPUDesc_.CheckEmitter(commandContext, transformAreaEmitterForGPUs_.size(), transformAreaEmitterForGPUs_.data());

		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 0, emitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 1, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 2, vertexEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 3, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 4, meshEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 5, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 6, transformModelEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 7, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 8, transformAreaEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 9, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());

		commandContext.SetComputeConstantBuffer(QueueType::Type::SPAWN, 10, emitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::SPAWN, 11, vertexEmitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::SPAWN, 12, meshEmitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::SPAWN, 13, transformModelEmitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::SPAWN, 14, transformAreaEmitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());

		commandContext.Dispatch(QueueType::Type::SPAWN, uint32_t(sumEmitter), 1, 1);

		commandContext.UAVBarrier(QueueType::Type::SPAWN, emitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, emitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, vertexEmitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, vertexEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, meshEmitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, meshEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, transformModelEmitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, transformModelEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, transformAreaEmitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, transformAreaEmitterForGPUDesc_.originalBuffer);
		//commandContext.SetMarker("Marker");
		commandContext.EndEvent(QueueType::Type::SPAWN);
	}
}

void GPUParticle::AddEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty() ||
		!vertexEmitterForGPUs_.empty() ||
		!meshEmitterForGPUs_.empty() ||
		!transformModelEmitterForGPUs_.empty() ||
		!transformAreaEmitterForGPUs_.empty()) {

		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"AddEmitter");

		emitterForGPUDesc_.AddEmitter(commandContext);
		vertexEmitterForGPUDesc_.AddEmitter(commandContext);
		meshEmitterForGPUDesc_.AddEmitter(commandContext);
		transformModelEmitterForGPUDesc_.AddEmitter(commandContext);
		transformAreaEmitterForGPUDesc_.AddEmitter(commandContext);

		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 0, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 1, emitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 2, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 3, vertexEmitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 4, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 5, meshEmitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 6, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 7, transformModelEmitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 8, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 9, transformAreaEmitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());


		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 10, emitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 11, vertexEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 12, meshEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 13, transformModelEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::SPAWN, 14, transformAreaEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.Dispatch(QueueType::Type::SPAWN, 1, 1, 1);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, emitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, emitterForGPUDesc_.createEmitterBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, vertexEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, vertexEmitterForGPUDesc_.createEmitterBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, meshEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, meshEmitterForGPUDesc_.createEmitterBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, transformModelEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, transformModelEmitterForGPUDesc_.createEmitterBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, transformAreaEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(QueueType::Type::SPAWN, transformAreaEmitterForGPUDesc_.createEmitterBuffer);
		emitterForGPUs_.clear();
		vertexEmitterForGPUs_.clear();
		meshEmitterForGPUs_.clear();
		transformModelEmitterForGPUs_.clear();
		emitterForGPUs_.clear();
		transformAreaEmitterForGPUs_.clear();

		commandContext.EndEvent(QueueType::Type::SPAWN);
	}
}

void GPUParticle::UpdateEmitter(CommandContext& commandContext) {
	// createParticleのリセット
	commandContext.BeginEvent(QueueType::Type::SPAWN, L"UpdateEmitter");
	commandContext.CopyBufferRegion(QueueType::Type::SPAWN, sumCreateParticleCounterBuffer_, 0, resetCounterBuffer_, 0, sizeof(UINT));
	commandContext.CopyBufferRegion(QueueType::Type::SPAWN, createParticleBuffer_, createParticleBuffer_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(UINT));

	commandContext.TransitionResource(QueueType::Type::SPAWN, createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, sumCreateParticleCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	emitterForGPUDesc_.UpdateEmitter(commandContext);
	vertexEmitterForGPUDesc_.UpdateEmitter(commandContext);
	meshEmitterForGPUDesc_.UpdateEmitter(commandContext);
	transformModelEmitterForGPUDesc_.UpdateEmitter(commandContext);
	transformAreaEmitterForGPUDesc_.UpdateEmitter(commandContext);

	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 0, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 1, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 2, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 3, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 4, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());

	commandContext.SetComputeDescriptorTable(QueueType::Type::SPAWN, 5, createParticleBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 6, sumCreateParticleCounterBuffer_->GetGPUVirtualAddress());

	commandContext.Dispatch(QueueType::Type::SPAWN, 1, 1, 1);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, createParticleBuffer_);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, emitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, vertexEmitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, meshEmitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, transformModelEmitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, transformAreaEmitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, sumCreateParticleCounterBuffer_);
	// x
	// 合計何個のパーティクルを生成するのか
	commandContext.CopyBufferRegion(QueueType::Type::SPAWN, spawnArgumentBuffer_, 0, sumCreateParticleCounterBuffer_, 0, sizeof(UINT));
	// y
	// エミッターの数
	commandContext.CopyBufferRegion(QueueType::Type::SPAWN, spawnArgumentBuffer_, sizeof(UINT), createParticleBuffer_, createParticleBuffer_.GetCounterOffset(), sizeof(UINT));
	commandContext.EndEvent(QueueType::Type::SPAWN);

	//// emitter系の書き込み終了を伝える
	//auto graphics = GraphicsCore::GetInstance();
	//auto& directQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::DIRECT));
	//auto& computeQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::COMPUTE));
	//auto& directQueueFence = graphics->GetCommandListManager().GetDirectQueueFence();
	//auto& computeQueueFence = graphics->GetCommandListManager().GetComputeQueueFence();
	//
	//auto queueType = QueueType::Type::Param::COMPUTE;
	//computeQueue.Signal(computeQueueFence.fence.Get(), computeQueueFence.fenceValue, QueueType::GetTypeString(queueType));
	//queueType = QueueType::Type::Param::DIRECT;
	//directQueue.Wait(computeQueueFence.fence.Get(), computeQueueFence.fenceValue, QueueType::GetTypeString(queueType));
}

void GPUParticle::Spawn(CommandContext& commandContext, const UploadBuffer& random) {
	commandContext.BeginEvent(QueueType::Type::SPAWN, L"Spawn");
	// あと何個生成できるかコピー
	directParticle_.TransitionResource(QueueType::Type::SPAWN, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, commandContext);
	computeParticle_.TransitionResource(QueueType::Type::SPAWN, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, commandContext);
	commandContext.TransitionResource(QueueType::Type::SPAWN, createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, trailsStockBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, trailsDataBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, trailsHeadBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, spawnArgumentBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	emitterForGPUDesc_.Spawn(commandContext);
	vertexEmitterForGPUDesc_.Spawn(commandContext);
	meshEmitterForGPUDesc_.Spawn(commandContext);
	transformModelEmitterForGPUDesc_.Spawn(commandContext);
	transformAreaEmitterForGPUDesc_.Spawn(commandContext);

	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 0, directParticle_.buffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 1, directParticle_.freeList.list->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 2, directParticle_.freeList.index->GetGPUVirtualAddress());

	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 3, computeParticle_.buffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 4, computeParticle_.freeList.list->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 5, computeParticle_.freeList.index->GetGPUVirtualAddress());

	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 6, createParticleBuffer_->GetGPUVirtualAddress());

	commandContext.SetComputeDescriptorTable(QueueType::Type::SPAWN, 7, trailsStockBuffers_.GetUAVHandle());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 8, trailsDataBuffers_->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::SPAWN, 9, trailsHeadBuffers_->GetGPUVirtualAddress());

	commandContext.SetComputeShaderResource(QueueType::Type::SPAWN, 10, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::SPAWN, 11, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::SPAWN, 12, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::SPAWN, 13, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::SPAWN, 14, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());

	commandContext.SetComputeDescriptorTable(QueueType::Type::SPAWN, 15, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());
	commandContext.SetComputeDescriptorTable(QueueType::Type::SPAWN, 16, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());

	commandContext.SetComputeConstantBuffer(QueueType::Type::SPAWN, 17, random.GetGPUVirtualAddress());

	commandContext.ExecuteIndirect(
		*spawnCommandSignature_,
		1,
		spawnArgumentBuffer_,
		0,
		nullptr,
		0,
		QueueType::Type::SPAWN
	);
	directParticle_.UavBarrier(QueueType::Type::SPAWN, commandContext);
	computeParticle_.UavBarrier(QueueType::Type::SPAWN, commandContext);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, createParticleBuffer_);

	commandContext.UAVBarrier(QueueType::Type::SPAWN, trailsStockBuffers_);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, trailsDataBuffers_);
	commandContext.UAVBarrier(QueueType::Type::SPAWN, trailsHeadBuffers_);

	commandContext.CopyBufferRegion(QueueType::Type::SPAWN, createParticleBuffer_, createParticleBuffer_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(UINT));

	// いったん閉じる
	commandContext.Close(QueueType::Type::Param::SPAWN);
	// ParticleBuffer系の書き込み終了を伝える
	auto graphics = GraphicsCore::GetInstance();
	auto& directQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::DIRECT));
	auto& computeQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::COMPUTE));
	auto& directQueueFence = graphics->GetCommandListManager().GetDirectQueueFence();
	auto& computeQueueFence = graphics->GetCommandListManager().GetComputeQueueFence();

	auto queueType = QueueType::Type::Param::COMPUTE;

	computeQueue.Signal(computeQueueFence.fence.Get(), computeQueueFence.fenceValue, QueueType::GetTypeString(queueType));
	computeQueue.Wait(computeQueueFence.fence.Get(), computeQueueFence.fenceValue, QueueType::GetTypeString(queueType));
	queueType = QueueType::Type::Param::DIRECT;
	directQueue.Wait(computeQueueFence.fence.Get(), computeQueueFence.fenceValue, QueueType::GetTypeString(queueType));
	queueType = QueueType::Type::Param::SPAWN;


	computeQueue.ExecuteCommandList(commandContext.GetCurrentCommandList(QueueType::Type::Param::SPAWN), QueueType::GetTypeString(queueType));
	commandContext.ResetCommandList(QueueType::Type::Param::SPAWN);

	commandContext.EndEvent(QueueType::Type::COMPUTE);
}

// 描画に必要なパーティクル数をリセット
//commandContext.CopyBufferRegion(QueueType::Type::DIRECT, appendDrawIndexBuffers_, appendDrawIndexBuffers_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(UINT));

void GPUParticle::ParticleUpdate(const ViewProjection& viewProjection, CommandContext& commandContext) {
	commandContext.BeginEvent(QueueType::Type::DIRECT, L"ParticleUpdate");

	UpdateDirectParticle(viewProjection, commandContext);
	UpdateComputeParticle(viewProjection, commandContext);

	commandContext.EndEvent(QueueType::Type::DIRECT);
}

void GPUParticle::UpdateTrails(const ViewProjection& viewProjection, CommandContext& commandContext) {
	commandContext.BeginEvent(QueueType::Type::COMPUTE, L"UpdateTrails");

	commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, trailsIndexBuffers_, trailsIndexBuffers_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(int32_t));

	commandContext.TransitionResource(QueueType::Type::COMPUTE, trailsStockBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, trailsIndexBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, trailsDataBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, trailsPositionBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//commandContext.TransitionResource(QueueType::Type::COMPUTE, particleBuffer_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 0, trailsStockBuffers_.GetUAVHandle());
	commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 1, trailsIndexBuffers_.GetUAVHandle());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, trailsDataBuffers_.GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 3, trailsPositionBuffers_.GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 4, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 5, sizeof(ConstBufferDataViewProjection), viewProjection.constMap_);

	commandContext.Dispatch(QueueType::Type::COMPUTE, static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxTrailsNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(QueueType::Type::COMPUTE, trailsStockBuffers_);
	commandContext.UAVBarrier(QueueType::Type::COMPUTE, trailsIndexBuffers_);
	commandContext.UAVBarrier(QueueType::Type::COMPUTE, trailsDataBuffers_);
	commandContext.UAVBarrier(QueueType::Type::COMPUTE, trailsPositionBuffers_);
	commandContext.EndEvent(QueueType::Type::COMPUTE);
}

void GPUParticle::AddTrailsVertex(CommandContext& commandContext) {
	commandContext.BeginEvent(QueueType::Type::COMPUTE, L"AddTrailsVertex");

	commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, trailsVertexDataBuffers_, trailsVertexDataBuffers_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(int32_t));

	commandContext.TransitionResource(QueueType::Type::COMPUTE, trailsVertexDataBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, trailsDataBuffers_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, trailsPositionBuffers_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 0, trailsVertexDataBuffers_.GetUAVHandle());
	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 1, trailsDataBuffers_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 2, trailsPositionBuffers_.GetGPUVirtualAddress());

	commandContext.Dispatch(QueueType::Type::COMPUTE, static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxTrailsNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(QueueType::Type::COMPUTE, trailsVertexDataBuffers_);
	commandContext.EndEvent(QueueType::Type::COMPUTE);
}

void GPUParticle::BulletUpdate(CommandContext& commandContext, const UploadBuffer& random) {
	if (!bullets_.empty()) {
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"UpdateBullet");
		size_t size = bullets_.size();
		bulletCountBuffer_.Copy(&size, sizeof(UINT));

		bulletsBuffer_.Copy(bullets_.data(), sizeof(GPUParticleShaderStructs::BulletForGPU) * size);

		commandContext.TransitionResource(QueueType::Type::COMPUTE, directParticle_.buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, computeParticle_.buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, bulletsBuffer_.GetBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 0, bulletsBuffer_.GetSRV());
		commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 1, bulletCountBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, directParticle_.buffer.GetGPUVirtualAddress());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 3, computeParticle_.buffer.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 4, random.GetGPUVirtualAddress());
		commandContext.Dispatch(QueueType::Type::COMPUTE, static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, directParticle_.buffer);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, computeParticle_.buffer);
		bullets_.clear();
		commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	//commandContext.BeginEvent(QueueType::Type::DIRECT, L"DrawParticle");
	//UINT64 destInstanceCountArgumentOffset = sizeof(GPUParticleShaderStructs::IndirectCommand::SRV) + sizeof(UINT);
	//UINT64 aliveDirectParticleCount = appendAliveDirectParticleBuffer_.GetCounterOffset();
	//UINT64 aliveComputeParticleCount = appendAliveComputeParticleBuffer_.GetCounterOffset();
	//UINT64 sumAliveParticleCount = aliveDirectParticleCount + aliveComputeParticleCount;

	//commandContext.CopyBufferRegion(QueueType::Type::DIRECT, drawArgumentBuffer_, destInstanceCountArgumentOffset, sizeof(UINT), &sumAliveParticleCount);
	//// 残りのパーティクルを描画
	////commandContext.CopyBufferRegion(drawIndexCountBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));
	//// 出ているパーティクルを描画
	//commandContext.ReadBackCopyBufferRegion(QueueType::Type::DIRECT, drawIndexCountBuffer_, 0, drawIndexCommandBuffers_, drawIndexCommandBuffers_.GetCounterOffset(), sizeof(UINT));

	//commandContext.TransitionResource(QueueType::Type::DIRECT, particleBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(QueueType::Type::DIRECT, drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(QueueType::Type::DIRECT, drawArgumentBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	//commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//commandContext.SetGraphicsConstantBuffer(2, viewProjection.constBuff_.GetGPUVirtualAddress());
	//commandContext.SetGraphicsDescriptorTable(3, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());
	//commandContext.SetGraphicsDescriptorTable(4, SamplerManager::LinearWrap);
	//commandContext.ExecuteIndirect(
	//	*commandSignature_,
	//	1,
	//	drawArgumentBuffer_,
	//	0,
	//	nullptr,
	//	0,
	//	QueueType::Type::DIRECT
	//);
	//// ComputeQueueで使用するので戻しておく
	//commandContext.TransitionResource(QueueType::Type::DIRECT, particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//commandContext.TransitionResource(QueueType::Type::DIRECT, drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//commandContext.EndEvent(QueueType::Type::DIRECT);
}

void GPUParticle::DrawTrails(const ViewProjection& viewProjection, CommandContext& commandContext) {
	//	commandContext.BeginEvent(QueueType::Type::DIRECT, L"DrawTrails");
	//	size_t instanceCount = sizeof(GPUParticleShaderStructs::TrailsCommand::SRV) + sizeof(UINT);
	//	commandContext.CopyBufferRegion(QueueType::Type::DIRECT, trailsArgumentBuffers_, instanceCount, trailsVertexDataBuffers_, trailsVertexDataBuffers_.GetCounterOffset(), sizeof(UINT));
	//	commandContext.CopyBufferRegion(QueueType::Type::DIRECT, trailsDrawInstanceCountBuffers_, 0, trailsVertexDataBuffers_, trailsVertexDataBuffers_.GetCounterOffset(), sizeof(UINT));
	//
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsDataBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsPositionBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsVertexDataBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsDrawInstanceCountBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsArgumentBuffers_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	//
	//	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//commandContext.SetGraphicsConstantBuffer(5, viewProjection.constBuff_.GetGPUVirtualAddress());
	//	commandContext.SetGraphicsDescriptorTable(6, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());
	//	commandContext.SetGraphicsDescriptorTable(7, SamplerManager::LinearWrap);
	//	commandContext.ExecuteIndirect(
	//		*trailsDrawCommandSignature_,
	//		1,
	//		trailsArgumentBuffers_,
	//		0,
	//		nullptr,
	//		0,
	//		QueueType::Type::DIRECT
	//	);
	//	// COMPUTEで使うように戻す
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsDataBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsPositionBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsVertexDataBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//	commandContext.TransitionResource(QueueType::Type::DIRECT, trailsDrawInstanceCountBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//	commandContext.EndEvent(QueueType::Type::DIRECT);
}

void GPUParticle::DrawImGui() {
	//	ImGui::Begin("GPUParticle");
	//	ImGui::Text("MaxParticleNum:%d", GPUParticleShaderStructs::MaxParticleNum);
	//	ImGui::Text("CurrentParticleNum:%d", *static_cast<uint32_t*>(drawIndexCountBuffer_.GetCPUData()));
	//	ImGui::End();
}

void GPUParticle::CreateMeshParticle(const ModelHandle& modelHandle, Engine::Animation::Animation& animation, const Matrix4x4& worldMatrix, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"MeshParticle");
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		/*commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

		commandContext.TransitionResource(QueueType::Type::COMPUTE, particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		*/commandContext.TransitionResource(QueueType::Type::COMPUTE, animation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, model.GetIndexBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		/*commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 1, originalCommandBuffer_.GetUAVHandle());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, originalCounterBuffer_.GetGPUVirtualAddress());
		*/commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 3, animation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 4, model.GetIndexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 5, random.GetGPUVirtualAddress());
		ConstBufferDataWorldTransform constBufferDataWorldTransform{};
		constBufferDataWorldTransform.matWorld = worldMatrix;
		constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldMatrix));
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 6, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);
		size_t indexCount = model.GetAllIndexCount();
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 7, sizeof(indexCount), &indexCount);
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 8, sizeof(mesh), &mesh);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(QueueType::Type::COMPUTE, UINT(numThreadGroups), createParticleNum, 1);
		/*commandContext.UAVBarrier(QueueType::Type::COMPUTE, originalCommandBuffer_);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, particleBuffer_);
		*/commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::CreateMeshParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"MeshParticle");
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		/*commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

		commandContext.TransitionResource(QueueType::Type::COMPUTE, particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		*/commandContext.TransitionResource(QueueType::Type::COMPUTE, model.GetVertexBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, model.GetIndexBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		/*commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 1, originalCommandBuffer_.GetUAVHandle());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, originalCounterBuffer_.GetGPUVirtualAddress());
		*/commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 3, model.GetVertexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 4, model.GetIndexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 5, random.GetGPUVirtualAddress());
		ConstBufferDataWorldTransform constBufferDataWorldTransform{};
		constBufferDataWorldTransform.matWorld = worldTransform;
		constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 6, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);

		size_t indexCount = model.GetAllIndexCount();
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 7, sizeof(indexCount), &indexCount);
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 8, sizeof(mesh), &mesh);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(QueueType::Type::COMPUTE, UINT(numThreadGroups), createParticleNum, 1);
		/*commandContext.UAVBarrier(QueueType::Type::COMPUTE, originalCommandBuffer_);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, particleBuffer_);
		*/commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::CreateVertexParticle(const ModelHandle& modelHandle, Engine::Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	commandContext.BeginEvent(QueueType::Type::COMPUTE, L"VertexParticle");
	auto& model = ModelManager::GetInstance()->GetModel(modelHandle);

	/*	commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

		commandContext.TransitionResource(QueueType::Type::COMPUTE, particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, animation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 1, originalCommandBuffer_.GetUAVHandle());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, originalCounterBuffer_.GetGPUVirtualAddress());*/
	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 3, animation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 4, random.GetGPUVirtualAddress());
	ConstBufferDataWorldTransform constBufferDataWorldTransform{};
	constBufferDataWorldTransform.matWorld = worldTransform;
	constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 5, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);
	size_t vertexCount = model.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 6, sizeof(vertexCount), &vertexCount);
	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 7, sizeof(mesh), &mesh);

	size_t numThreadGroups = (vertexCount + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;

	commandContext.Dispatch(QueueType::Type::COMPUTE, UINT(numThreadGroups), 1, 1);
	/*	commandContext.UAVBarrier(QueueType::Type::COMPUTE, originalCommandBuffer_);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, particleBuffer_);*/
	commandContext.EndEvent(QueueType::Type::COMPUTE);
}

void GPUParticle::CreateVertexParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	commandContext.BeginEvent(QueueType::Type::COMPUTE, L"VertexParticle");

	auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
	// あと何個生成できるかコピー
	/*commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

	commandContext.TransitionResource(QueueType::Type::COMPUTE, particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, originalCounterBuffer_.GetGPUVirtualAddress());*/
	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 3, model.GetVertexBuffer().GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 4, random.GetGPUVirtualAddress());
	ConstBufferDataWorldTransform constBufferDataWorldTransform{};
	constBufferDataWorldTransform.matWorld = worldTransform;
	constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));

	size_t vertexCount = model.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 5, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);
	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 6, sizeof(vertexCount), &vertexCount);
	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 7, sizeof(mesh), &mesh);

	size_t numThreadGroups = (vertexCount + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;

	commandContext.Dispatch(QueueType::Type::COMPUTE, UINT(numThreadGroups), 1, 1);
	/*	commandContext.UAVBarrier(QueueType::Type::COMPUTE, originalCommandBuffer_);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, particleBuffer_);*/
	commandContext.EndEvent(QueueType::Type::COMPUTE);
}

void GPUParticle::CreateEdgeParticle(const ModelHandle& modelHandle, Engine::Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"EdgeParticle");

		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		/*	commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

			commandContext.TransitionResource(QueueType::Type::COMPUTE, particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);*/
		commandContext.TransitionResource(QueueType::Type::COMPUTE, animation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, model.GetIndexBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		//commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, particleBuffer_.GetGPUVirtualAddress());
		//commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 1, originalCommandBuffer_.GetUAVHandle());
		//commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, originalCounterBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 3, animation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 4, model.GetIndexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 5, random.GetGPUVirtualAddress());
		ConstBufferDataWorldTransform constBufferDataWorldTransform{};
		constBufferDataWorldTransform.matWorld = worldTransform;
		constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 6, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);

		size_t indexCount = model.GetAllIndexCount();
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 7, sizeof(indexCount), &indexCount);
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 8, sizeof(mesh), &mesh);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		//uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(QueueType::Type::COMPUTE, UINT(numThreadGroups), 1, 1);
		/*commandContext.UAVBarrier(QueueType::Type::COMPUTE, originalCommandBuffer_);*/
		commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::CreateEdgeParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.BeginEvent(QueueType::Type::COMPUTE, L"EdgeParticle");
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);

		/*commandContext.CopyBufferRegion(QueueType::Type::COMPUTE, originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

		commandContext.TransitionResource(QueueType::Type::COMPUTE, particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		*/commandContext.TransitionResource(QueueType::Type::COMPUTE, model.GetVertexBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.TransitionResource(QueueType::Type::COMPUTE, model.GetIndexBuffer(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		/*commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 1, originalCommandBuffer_.GetUAVHandle());
		commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, originalCounterBuffer_.GetGPUVirtualAddress());
		*/commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 3, model.GetVertexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 4, model.GetIndexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(QueueType::Type::COMPUTE, 5, random.GetGPUVirtualAddress());
		ConstBufferDataWorldTransform constBufferDataWorldTransform{};
		constBufferDataWorldTransform.matWorld = worldTransform;
		constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 6, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);

		uint32_t indexCount = model.GetAllIndexCount();
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 7, sizeof(indexCount), &indexCount);
		commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 8, sizeof(mesh), &mesh);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		//uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(QueueType::Type::COMPUTE, UINT(numThreadGroups), 1, 1);
		/*commandContext.UAVBarrier(QueueType::Type::COMPUTE, originalCommandBuffer_);
		commandContext.UAVBarrier(QueueType::Type::COMPUTE, particleBuffer_);
		*/commandContext.EndEvent(QueueType::Type::COMPUTE);
	}
}

void GPUParticle::SetField(const GPUParticleShaderStructs::FieldForCPU& fieldForCPU) {
	GPUParticleShaderStructs::FieldForGPU fieldForGPU{};
	GPUParticleShaderStructs::Copy(fieldForGPU, fieldForCPU);
	fields_.emplace_back(fieldForGPU);
}

void GPUParticle::SetEmitter(const GPUParticleShaderStructs::EmitterForCPU& emitterForCPU, const Matrix4x4& parent) {
	GPUParticleShaderStructs::EmitterForGPU emitterForGPU{};
	GPUParticleShaderStructs::Copy(emitterForGPU, emitterForCPU, parent);
	emitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetEmitter(const GPUParticleShaderStructs::VertexEmitterForCPU& emitterForCPU, const Matrix4x4& parent) {
	GPUParticleShaderStructs::VertexEmitterForGPU emitterForGPU{};
	GPUParticleShaderStructs::Copy(emitterForGPU, emitterForCPU, parent);
	vertexEmitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetEmitter(const GPUParticleShaderStructs::MeshEmitterForCPU& emitterForCPU, const Matrix4x4& parent) {
	GPUParticleShaderStructs::MeshEmitterForGPU emitterForGPU{};
	GPUParticleShaderStructs::Copy(emitterForGPU, emitterForCPU, parent);
	meshEmitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetEmitter(const GPUParticleShaderStructs::TransformModelEmitterForCPU& emitterForCPU, const Matrix4x4& parent) {
	GPUParticleShaderStructs::TransformModelEmitterForGPU emitterForGPU{};
	GPUParticleShaderStructs::Copy(emitterForGPU, emitterForCPU, parent);
	transformModelEmitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetEmitter(const GPUParticleShaderStructs::TransformAreaEmitterForCPU& emitterForCPU, const Matrix4x4& parent) {
	GPUParticleShaderStructs::TransformAreaEmitterForGPU emitterForGPU{};
	GPUParticleShaderStructs::Copy(emitterForGPU, emitterForCPU, parent);
	transformAreaEmitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetBullet(const GPUParticleShaderStructs::BulletForGPU& bullet) {
	bullets_.emplace_back(bullet);
}

void GPUParticle::InitializeParticleBuffer() {
	size_t size = sizeof(GPUParticleShaderStructs::Particle) * GPUParticleShaderStructs::DivisionParticleNum;
	directParticle_.Create(size, GPUParticleShaderStructs::DivisionParticleNum, L"directParticle");
	appendAliveDirectParticleBuffer_.Create(L"appendAliveDirectParticleBuffer", size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	// UAVView生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::DivisionParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = appendAliveDirectParticleBuffer_.GetCounterOffset();
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	appendAliveDirectParticleBuffer_.CreateUAV(uavDesc);

	computeParticle_.Create(size, GPUParticleShaderStructs::DivisionParticleNum, L"computeParticle");
	appendAliveComputeParticleBuffer_.Create(L"appendAliveComputeParticleBuffer", size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::DivisionParticleNum;
	uavDesc.Buffer.CounterOffsetInBytes = appendAliveComputeParticleBuffer_.GetCounterOffset();
	appendAliveComputeParticleBuffer_.CreateUAV(uavDesc);


	size = sizeof(GPUParticleShaderStructs::DrawIndex) * GPUParticleShaderStructs::MaxParticleNum;
	appendDrawIndexBuffers_.Create(L"appendDrawIndexBuffers", size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	// UAVView生成
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::DrawIndex);
	uavDesc.Buffer.CounterOffsetInBytes = appendDrawIndexBuffers_.GetCounterOffset();
	appendDrawIndexBuffers_.CreateUAV(uavDesc);
}

void GPUParticle::InitializeUpdateParticle() {
	drawIndexCountBuffer_.Create(L"DrawIndexCountBuffer", sizeof(UINT));
	drawIndexCountBuffer_.ResetBuffer();

	// Draw引数用バッファー
	drawArgumentBuffer_.Create(
		L"DrawArgumentBuffer",
		sizeof(GPUParticleShaderStructs::IndirectCommand), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
}

void GPUParticle::InitializeSpawnBuffer() {
	/*auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();*/
	// spawn引数用バッファー
	spawnArgumentBuffer_.Create(
		L"SpawnArgumentBuffer",
		sizeof(D3D12_DISPATCH_ARGUMENTS)
	);
}

void GPUParticle::InitializeBuffer() {
	enum {
		kDirectParticleFreeList,
		kDirectParticleIndex,
		kComputeParticleFreeList,
		kComputeParticleIndex,
		kFieldeParticleFreeList,
		kFieldeParticleIndex,
		kDrawIndex,
		kTrailsIndexBuffer,

		kCount,
	};
	CommandContext commandContext;
	commandContext.Create();
	//commandContext.Close();
	commandContext.Start();
	// スポーンシグネイチャー
	{
		initializeBufferRootSignature_ = std::make_unique<RootSignature>();
		//	createParticle用
		CD3DX12_DESCRIPTOR_RANGE trailsIndexBuffer[1]{};
		trailsIndexBuffer[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, kTrailsIndexBuffer, 0);
		CD3DX12_ROOT_PARAMETER rootParameters[kCount]{};
		rootParameters[kDirectParticleFreeList].InitAsUnorderedAccessView(0);
		rootParameters[kDirectParticleIndex].InitAsUnorderedAccessView(1);
		rootParameters[kComputeParticleFreeList].InitAsUnorderedAccessView(2);
		rootParameters[kComputeParticleIndex].InitAsUnorderedAccessView(3);
		rootParameters[kFieldeParticleFreeList].InitAsUnorderedAccessView(4);
		rootParameters[kFieldeParticleIndex].InitAsUnorderedAccessView(5);
		rootParameters[kDrawIndex].InitAsUnorderedAccessView(6);
		rootParameters[kTrailsIndexBuffer].InitAsDescriptorTable(_countof(trailsIndexBuffer), trailsIndexBuffer);
		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		initializeBufferRootSignature_->Create(L"InitializeBufferRootSignature", desc);
	}
	// スポーンパイプライン
	{
		initializeBufferPipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *initializeBufferRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/InitializeGPUParticle.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		initializeBufferPipelineState_->Create(L"InitializeBufferCPSO", desc);
	}

	commandContext.SetComputeRootSignature(QueueType::Type::COMPUTE, *initializeBufferRootSignature_);
	commandContext.SetPipelineState(QueueType::Type::COMPUTE, *initializeBufferPipelineState_);

	commandContext.TransitionResource(QueueType::Type::COMPUTE, directParticle_.freeList.list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, directParticle_.freeList.index, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, computeParticle_.freeList.list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, computeParticle_.freeList.index, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, fieldBuffer_.freeList.list, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::COMPUTE, fieldBuffer_.freeList.index, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.TransitionResource(QueueType::Type::COMPUTE, trailsStockBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, directParticle_.freeList.list->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 1, directParticle_.freeList.index->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, computeParticle_.freeList.list->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 3, computeParticle_.freeList.index->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 4, fieldBuffer_.freeList.list->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 5, fieldBuffer_.freeList.index->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 6, appendDrawIndexBuffers_->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, kTrailsIndexBuffer, trailsStockBuffers_.GetUAVHandle());
	commandContext.Dispatch(QueueType::Type::COMPUTE, static_cast<UINT>(ceil(GPUParticleShaderStructs::MaxParticleNum / float(GPUParticleShaderStructs::ComputeThreadBlockSize))), 1, 1);

	// エミッター初期化
	{
		emitterForGPUDesc_.Initialize(commandContext);
		vertexEmitterForGPUDesc_.Initialize(commandContext);
		meshEmitterForGPUDesc_.Initialize(commandContext);
		transformModelEmitterForGPUDesc_.Initialize(commandContext);
		transformAreaEmitterForGPUDesc_.Initialize(commandContext);
	}
	// フィールド初期化
	{
		size_t size = sizeof(GPUParticleShaderStructs::FieldForGPU) * GPUParticleShaderStructs::MaxFieldNum;
		commandContext.ResetBuffer(QueueType::Type::COMPUTE, fieldAddBuffer_, size);
		commandContext.ResetBuffer(QueueType::Type::COMPUTE, fieldBuffer_.buffer, size);
	}

	{
		GPUParticleShaderStructs::IndirectCommand tmp{};
		tmp.srv.directParticleSRV = directParticle_.buffer.GetGPUVirtualAddress();
		tmp.srv.computeParticleSRV = computeParticle_.buffer.GetGPUVirtualAddress();
		tmp.srv.drawIndexSRV = appendDrawIndexBuffers_->GetGPUVirtualAddress();
		tmp.drawIndex.IndexCountPerInstance = UINT(6);
		tmp.drawIndex.InstanceCount = 1;
		tmp.drawIndex.BaseVertexLocation = 0;
		tmp.drawIndex.StartIndexLocation = 0;
		tmp.drawIndex.StartInstanceLocation = 0;
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, drawArgumentBuffer_, sizeof(GPUParticleShaderStructs::IndirectCommand), &tmp);

	}

	{
		GPUParticleShaderStructs::TrailsCommand tmp{};
		tmp.srv.counterBuffer = trailsIndexBuffers_->GetGPUVirtualAddress();
		tmp.srv.trailsData = trailsDataBuffers_->GetGPUVirtualAddress();
		tmp.srv.trailsPosition = trailsPositionBuffers_->GetGPUVirtualAddress();
		tmp.srv.vertexBuffer = trailsVertexDataBuffers_->GetGPUVirtualAddress();
		tmp.srv.instanceCount = trailsDrawInstanceCountBuffers_->GetGPUVirtualAddress();
		tmp.drawIndex.VertexCountPerInstance = 3;
		tmp.drawIndex.InstanceCount = 1;
		tmp.drawIndex.StartInstanceLocation = 0;
		tmp.drawIndex.StartVertexLocation = 0;
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, trailsArgumentBuffers_, sizeof(GPUParticleShaderStructs::TrailsCommand), &tmp);

	}

	{
		D3D12_DISPATCH_ARGUMENTS tmp{};
		tmp.ThreadGroupCountX = 1;
		tmp.ThreadGroupCountY = 1;
		tmp.ThreadGroupCountZ = 1;
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, spawnArgumentBuffer_, sizeof(D3D12_DISPATCH_ARGUMENTS), &tmp);
	}
	commandContext.Close();
	commandContext.Flush();

}

void GPUParticle::InitializeEmitter() {

	createParticleBuffer_.Create(L"CreateParticleBuffer", sizeof(GPUParticleShaderStructs::CreateParticle) * GPUParticleShaderStructs::MaxEmitterNum);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxEmitterNum;
	uavDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::CreateParticle);
	uavDesc.Buffer.CounterOffsetInBytes = createParticleBuffer_.GetCounterOffset();
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	createParticleBuffer_.CreateUAV(uavDesc);

	sumCreateParticleCounterBuffer_.Create(L"createParticleCounterCopySrcBuffer", sizeof(uint32_t));
}

void GPUParticle::InitializeAddEmitter() {
	emitterForGPUDesc_.Create(L"Emitter", GPUParticleShaderStructs::EmitterType::kEmitter);
	vertexEmitterForGPUDesc_.Create(L"VertexEmitter", GPUParticleShaderStructs::EmitterType::kVertexEmitter);
	meshEmitterForGPUDesc_.Create(L"MeshEmitter", GPUParticleShaderStructs::EmitterType::kMeshEmitter);
	transformModelEmitterForGPUDesc_.Create(L"TransformModelEmitter", GPUParticleShaderStructs::EmitterType::kTransformModelEmitter);
	transformAreaEmitterForGPUDesc_.Create(L"TransformAreaEmitter", GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter);
}

void GPUParticle::InitializeBullets() {
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Buffer.NumElements = GPUParticleShaderStructs::MaxBulletNum;
	desc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::BulletForGPU);
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	bulletsBuffer_.Create(L"BulletsBuffer", sizeof(GPUParticleShaderStructs::BulletForGPU) * GPUParticleShaderStructs::MaxBulletNum, desc);
	bulletCountBuffer_.Create(L"BulletCountBuffer", sizeof(UINT));
}

void GPUParticle::InitializeField() {
	{
		size_t size = sizeof(GPUParticleShaderStructs::FieldForGPU) * GPUParticleShaderStructs::MaxFieldNum;
		fieldBuffer_.Create(size, GPUParticleShaderStructs::MaxFieldNum, L"FieldBuffer");

		fieldCPUBuffer_.Create(L"FieldCPUBuffer", size);
		fieldCPUBuffer_.ResetBuffer();
		fieldAddBuffer_.Create(L"FieldAddBuffer", size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		appendFieldIndexBuffer_.Create(L"AppendFieldIndexBuffer", sizeof(UINT) * GPUParticleShaderStructs::MaxFieldNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxFieldNum;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT);
		uavDesc.Buffer.CounterOffsetInBytes = appendFieldIndexBuffer_.GetCounterOffset();
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		appendFieldIndexBuffer_.CreateUAV(uavDesc);
		createFieldNumBuffer_.Create(L"CreateFieldNumBuffer", sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}
}

void GPUParticle::InitializeTrails() {
	{
		trailsArgumentBuffers_.Create(L"trailsArgumentBuffers", sizeof(GPUParticleShaderStructs::TrailsCommand));
		trailsStockBuffers_.Create(L"trailsStockBuffers", sizeof(int32_t) * GPUParticleShaderStructs::MaxTrailsNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		trailsIndexBuffers_.Create(L"trailsIndexBuffers", sizeof(GPUParticleShaderStructs::TrailsIndex) * GPUParticleShaderStructs::MaxTrailsNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxTrailsNum;
		uavDesc.Buffer.StructureByteStride = sizeof(int32_t);
		uavDesc.Buffer.CounterOffsetInBytes = trailsStockBuffers_.GetCounterOffset();
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		trailsStockBuffers_.CreateUAV(uavDesc);
		uavDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::TrailsIndex);
		uavDesc.Buffer.CounterOffsetInBytes = trailsIndexBuffers_.GetCounterOffset();
		trailsIndexBuffers_.CreateUAV(uavDesc);
		trailsDataBuffers_.Create(L"trailsDataBuffers", sizeof(GPUParticleShaderStructs::TrailsData) * GPUParticleShaderStructs::MaxTrailsNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		trailsHeadBuffers_.Create(L"trailsHeadBuffers_", sizeof(GPUParticleShaderStructs::TrailsHead), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		trailsPositionBuffers_.Create(L"trailsBuffers", sizeof(GPUParticleShaderStructs::TrailsPosition) * GPUParticleShaderStructs::MaxTrailsTotal, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		trailsVertexDataBuffers_.Create(L"trailsVertexBuffers", sizeof(GPUParticleShaderStructs::TrailsVertexData) * GPUParticleShaderStructs::MaxTrailsTotal, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		uavDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::TrailsVertexData);
		uavDesc.Buffer.CounterOffsetInBytes = trailsVertexDataBuffers_.GetCounterOffset();
		trailsVertexDataBuffers_.CreateUAV(uavDesc);
		//trailsVertexDataBuffers_.CreateVertexView(sizeof(GPUParticleShaderStructs::TrailsVertex));
		//trailsIndiesBuffers_.Create(L"trailsIndiesBuffers",sizeof(uint32_t)* GPUParticleShaderStructs::MaxTrailsTotal * 3, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		//trailsIndiesBuffers_.CreateIndexView(DXGI_FORMAT_R32_UINT);
		trailsDrawInstanceCountBuffers_.Create(L"trailsDrawInstanceCountBuffers", sizeof(UINT));
	}
}

void GPUParticle::UpdateDirectParticle(const ViewProjection& viewProjection, CommandContext& commandContext) {
	directParticle_.TransitionResource(QueueType::Type::DIRECT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, commandContext);

	emitterForGPUDesc_.UpdateParticle(QueueType::Type::DIRECT, commandContext);
	vertexEmitterForGPUDesc_.UpdateParticle(QueueType::Type::DIRECT, commandContext);
	meshEmitterForGPUDesc_.UpdateParticle(QueueType::Type::DIRECT, commandContext);
	transformModelEmitterForGPUDesc_.UpdateParticle(QueueType::Type::DIRECT, commandContext);
	transformAreaEmitterForGPUDesc_.UpdateParticle(QueueType::Type::DIRECT, commandContext);


	commandContext.SetComputeUAV(QueueType::Type::DIRECT, 0, directParticle_.buffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::DIRECT, 1, directParticle_.freeList.list->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::DIRECT, 2, directParticle_.freeList.index->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(QueueType::Type::DIRECT, 3, appendAliveDirectParticleBuffer_.GetUAVHandle());


	commandContext.SetComputeShaderResource(QueueType::Type::DIRECT, 4, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::DIRECT, 5, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::DIRECT, 6, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::DIRECT, 7, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::DIRECT, 8, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());


	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::DIRECT, 9, sizeof(ConstBufferDataViewProjection), viewProjection.constMap_);

	commandContext.Dispatch(QueueType::Type::DIRECT, static_cast<UINT>(ceil((GPUParticleShaderStructs::DivisionParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);

	directParticle_.UavBarrier(QueueType::Type::DIRECT, commandContext);
	commandContext.UAVBarrier(QueueType::Type::DIRECT, appendAliveDirectParticleBuffer_);
}

void GPUParticle::UpdateComputeParticle(const ViewProjection& viewProjection, CommandContext& commandContext) {
	computeParticle_.TransitionResource(QueueType::Type::COMPUTE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, commandContext);

	emitterForGPUDesc_.UpdateParticle(QueueType::Type::COMPUTE, commandContext);
	vertexEmitterForGPUDesc_.UpdateParticle(QueueType::Type::COMPUTE, commandContext);
	meshEmitterForGPUDesc_.UpdateParticle(QueueType::Type::COMPUTE, commandContext);
	transformModelEmitterForGPUDesc_.UpdateParticle(QueueType::Type::COMPUTE, commandContext);
	transformAreaEmitterForGPUDesc_.UpdateParticle(QueueType::Type::COMPUTE, commandContext);


	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 0, computeParticle_.buffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 1, computeParticle_.freeList.list->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(QueueType::Type::COMPUTE, 2, computeParticle_.freeList.index->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(QueueType::Type::COMPUTE, 3, appendAliveComputeParticleBuffer_.GetUAVHandle());


	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 4, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 5, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 6, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 7, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(QueueType::Type::COMPUTE, 8, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());


	commandContext.SetComputeDynamicConstantBufferView(QueueType::Type::COMPUTE, 9, sizeof(ConstBufferDataViewProjection), viewProjection.constMap_);

	commandContext.Dispatch(QueueType::Type::COMPUTE, static_cast<UINT>(ceil((GPUParticleShaderStructs::DivisionParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);

	computeParticle_.UavBarrier(QueueType::Type::COMPUTE, commandContext);
	commandContext.UAVBarrier(QueueType::Type::COMPUTE, appendAliveComputeParticleBuffer_);
}

void GPUParticle::EmitterDesc::Initialize(CommandContext& commandContext) {
	switch (type) {
	case GPUParticleShaderStructs::EmitterType::kEmitter:
	{
		defaultCopyBuffer.Clear(QueueType::Type::COMPUTE, commandContext);
		originalBuffer.Clear(QueueType::Type::COMPUTE, commandContext);
		size_t size = sizeof(GPUParticleShaderStructs::EmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::EmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, originalBuffer, size, reset.data());
	}

	break;
	case GPUParticleShaderStructs::EmitterType::kVertexEmitter:
	{
		size_t size = sizeof(GPUParticleShaderStructs::VertexEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::VertexEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, originalBuffer, size, reset.data());
	}
	break;
	case GPUParticleShaderStructs::EmitterType::kMeshEmitter:
	{
		size_t size = sizeof(GPUParticleShaderStructs::MeshEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::MeshEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, originalBuffer, size, reset.data());
	}
	break;
	case GPUParticleShaderStructs::EmitterType::kTransformModelEmitter:
	{
		size_t size = sizeof(GPUParticleShaderStructs::TransformModelEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::TransformModelEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, originalBuffer, size, reset.data());
	}
	break;
	case GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter:
	{
		size_t size = sizeof(GPUParticleShaderStructs::TransformAreaEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::TransformAreaEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(QueueType::Type::COMPUTE, originalBuffer, size, reset.data());
	}
	break;
	default:
		break;
	}
}

void GPUParticle::EmitterDesc::Create(const std::wstring& name, const GPUParticleShaderStructs::EmitterType& emitterType) {
	{
		size_t addEmitterSize = 0;
		switch (emitterType) {
		case GPUParticleShaderStructs::EmitterType::kEmitter:
		{
			this->type = emitterType;
			addEmitterSize = sizeof(GPUParticleShaderStructs::EmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kVertexEmitter:
		{
			this->type = emitterType;
			addEmitterSize = sizeof(GPUParticleShaderStructs::VertexEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kMeshEmitter:
		{
			this->type = emitterType;
			addEmitterSize = sizeof(GPUParticleShaderStructs::MeshEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kTransformModelEmitter:
		{
			this->type = emitterType;
			addEmitterSize = sizeof(GPUParticleShaderStructs::TransformModelEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter:
		{
			this->type = emitterType;
			addEmitterSize = sizeof(GPUParticleShaderStructs::TransformAreaEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		default:
			break;
		}
		addCountBuffer.Create(name + L":AddCounterBuffer", sizeof(INT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		createEmitterBuffer.Create(name + L":CreateEmitterBuffer", sizeof(INT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}
}

size_t GPUParticle::EmitterDesc::CheckEmitter(CommandContext& commandContext, size_t emitterCount, void* data) {
	commandContext.BeginEvent(QueueType::Type::SPAWN, L"CheckEmitterType");
	if (emitterCount != 0) {
		size_t copySize = 0;
		switch (type) {
		case GPUParticleShaderStructs::EmitterType::kEmitter:
			copySize = sizeof(GPUParticleShaderStructs::EmitterForGPU) * emitterCount;
			break;
		case GPUParticleShaderStructs::EmitterType::kVertexEmitter:
			copySize = sizeof(GPUParticleShaderStructs::VertexEmitterForGPU) * emitterCount;
			break;
		case GPUParticleShaderStructs::EmitterType::kMeshEmitter:
			copySize = sizeof(GPUParticleShaderStructs::MeshEmitterForGPU) * emitterCount;
			break;
		case GPUParticleShaderStructs::EmitterType::kTransformModelEmitter:
			copySize = sizeof(GPUParticleShaderStructs::TransformModelEmitterForGPU) * emitterCount;
			break;
		case GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter:
			copySize = sizeof(GPUParticleShaderStructs::TransformAreaEmitterForGPU) * emitterCount;
			break;
		default:
			break;
		}
		commandContext.CopyBuffer(QueueType::Type::SPAWN, defaultCopyBuffer, copySize, data);
	}
	else {
		switch (type) {
		case GPUParticleShaderStructs::EmitterType::kEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::EmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::EmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(QueueType::Type::SPAWN, defaultCopyBuffer, size, reset.data());
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kVertexEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::VertexEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::VertexEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(QueueType::Type::SPAWN, defaultCopyBuffer, size, reset.data());
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kMeshEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::MeshEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::MeshEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(QueueType::Type::SPAWN, defaultCopyBuffer, size, reset.data());
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kTransformModelEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::TransformModelEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::TransformModelEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(QueueType::Type::SPAWN, defaultCopyBuffer, size, reset.data());
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::TransformAreaEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::TransformAreaEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(QueueType::Type::SPAWN, defaultCopyBuffer, size, reset.data());
		}
		break;
		default:
			break;
		}
	}
	commandContext.CopyBuffer(QueueType::Type::SPAWN, addCountBuffer, sizeof(int32_t), &emitterCount);
	commandContext.TransitionResource(QueueType::Type::SPAWN, defaultCopyBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, originalBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, addCountBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandContext.EndEvent(QueueType::Type::SPAWN);
	return emitterCount;
}

void GPUParticle::EmitterDesc::AddEmitter(CommandContext& commandContext) {
	commandContext.BeginEvent(QueueType::Type::SPAWN, L"AddEmitterType");
	commandContext.CopyBuffer(QueueType::Type::SPAWN, createEmitterBuffer, addCountBuffer);

	commandContext.TransitionResource(QueueType::Type::SPAWN, defaultCopyBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, originalBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(QueueType::Type::SPAWN, createEmitterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.EndEvent(QueueType::Type::SPAWN);
}

void GPUParticle::EmitterDesc::UpdateEmitter(CommandContext& commandContext) {
	commandContext.TransitionResource(QueueType::Type::SPAWN, originalBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void GPUParticle::EmitterDesc::UpdateParticle(const QueueType::Type::Param& type, CommandContext& commandContext) {
	commandContext.TransitionResource(type, originalBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void GPUParticle::EmitterDesc::Spawn(CommandContext& commandContext) {
	commandContext.TransitionResource(QueueType::Type::SPAWN, originalBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void GPUParticle::FreeListBuffer::Create(size_t size, uint32_t listNum, const std::wstring& name) {
	buffer.Create(
		name + L"Buffer",
		size,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	freeList.list.Create(
		name + L"freelist",
		sizeof(uint32_t) * listNum,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	freeList.index.Create(
		name + L"freelistIndex",
		sizeof(int32_t),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
}

void GPUParticle::FreeListBuffer::UavBarrier(const QueueType::Type::Param& type, CommandContext& commandContext) {
	commandContext.UAVBarrier(type, buffer);
	commandContext.UAVBarrier(type, freeList.list);
	commandContext.UAVBarrier(type, freeList.index);
}

void GPUParticle::FreeListBuffer::TransitionResource(const QueueType::Type::Param& type, const D3D12_RESOURCE_STATES& newState, CommandContext& commandContext) {
	commandContext.TransitionResource(type, buffer, newState);
	commandContext.TransitionResource(type, freeList.list, newState);
	commandContext.TransitionResource(type, freeList.index, newState);
}
