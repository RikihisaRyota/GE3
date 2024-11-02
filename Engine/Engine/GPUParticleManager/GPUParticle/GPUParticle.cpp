#include "GPUParticle.h"

#include <array>

#include <d3dx12.h>

#include "imgui.h"

#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/GraphicsCore.h"
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
	InitializeParticleBuffer();
	InitializeUpdateParticle();
	InitializeSpawnBuffer();
	InitializeEmitter();
	InitializeAddEmitter();
	InitializeBullets();
	InitializeField();
	InitializeBuffer();
	InitializeTrails();
}

GPUParticle::~GPUParticle() {}

void GPUParticle::Initialize() {}

void GPUParticle::CheckField(CommandContext& commandContext) {
	// エミッター追加
	if (!fields_.empty()) {
		commandContext.BeginEvent(L"CheckField");
		size_t fieldCount = fields_.size();
		size_t copySize = sizeof(GPUParticleShaderStructs::FieldForCPU) * fieldCount;
		fieldCPUBuffer_.ResetBuffer();
		fieldCPUBuffer_.Copy(fields_.data(), copySize);
		commandContext.CopyBuffer(fieldAddBuffer_, fieldCPUBuffer_);
		fieldCounterBuffer_.Copy(&fieldCount, sizeof(UINT));

		commandContext.TransitionResource(fieldAddBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(fieldOriginalBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, fieldAddBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(1, fieldOriginalBuffer_->GetGPUVirtualAddress());
		commandContext.Dispatch(uint32_t(fieldCount), 1, 1);

		commandContext.UAVBarrier(fieldOriginalBuffer_);
		commandContext.EndEvent();
	}
}

void GPUParticle::AddField(CommandContext& commandContext) {
	// フィールド追加
	if (!fields_.empty()) {
		commandContext.BeginEvent(L"AddField");
		commandContext.CopyBufferRegion(createFieldNumBuffer_, 0, fieldCounterBuffer_, 0, sizeof(UINT));

		commandContext.TransitionResource(fieldAddBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(fieldOriginalBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(createFieldNumBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(fieldIndexStockBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeShaderResource(0, fieldAddBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(1, fieldOriginalBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(2, createFieldNumBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(3, fieldIndexStockBuffer_.GetUAVHandle());
		commandContext.Dispatch(1, 1, 1);
		commandContext.UAVBarrier(fieldOriginalBuffer_);
		commandContext.EndEvent();
	}
}

void GPUParticle::UpdateField(CommandContext& commandContext) {

	if (!fields_.empty()) {
		commandContext.BeginEvent(L"UpdateField");
		commandContext.CopyBufferRegion(fieldIndexBuffer_, fieldIndexBuffer_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(UINT));

		commandContext.TransitionResource(fieldOriginalBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(fieldIndexStockBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(fieldIndexBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, fieldOriginalBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, fieldIndexStockBuffer_.GetUAVHandle());
		commandContext.SetComputeDescriptorTable(2, fieldIndexBuffer_.GetUAVHandle());
		commandContext.Dispatch(1, 1, 1);
		commandContext.UAVBarrier(fieldOriginalBuffer_);
		commandContext.UAVBarrier(fieldIndexStockBuffer_);
		commandContext.UAVBarrier(fieldIndexBuffer_);
		commandContext.EndEvent();
	}
}

void GPUParticle::CollisionField(CommandContext& commandContext, const UploadBuffer& random) {
	if (!fields_.empty()) {
		commandContext.BeginEvent(L"CollisionField");
		commandContext.TransitionResource(fieldOriginalBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(fieldIndexBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeShaderResource(0, fieldOriginalBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(1, fieldIndexBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(2, particleBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(3, random.GetGPUVirtualAddress());
		commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), uint32_t(fields_.size()), 1);
		commandContext.UAVBarrier(particleBuffer_);
		fields_.clear();
		commandContext.EndEvent();
	}
}

void GPUParticle::CheckEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty() ||
		!vertexEmitterForGPUs_.empty() ||
		!meshEmitterForGPUs_.empty() ||
		!transformModelEmitterForGPUs_.empty() ||
		!transformAreaEmitterForGPUs_.empty()) {

		commandContext.BeginEvent(L"CheckEmitter");

		size_t sumEmitter = 0;
		sumEmitter += emitterForGPUDesc_.CheckEmitter(commandContext, emitterForGPUs_.size(), emitterForGPUs_.data());
		sumEmitter += vertexEmitterForGPUDesc_.CheckEmitter(commandContext, vertexEmitterForGPUs_.size(), vertexEmitterForGPUs_.data());
		sumEmitter += meshEmitterForGPUDesc_.CheckEmitter(commandContext, meshEmitterForGPUs_.size(), meshEmitterForGPUs_.data());
		sumEmitter += transformModelEmitterForGPUDesc_.CheckEmitter(commandContext, transformModelEmitterForGPUs_.size(), transformModelEmitterForGPUs_.data());
		sumEmitter += transformAreaEmitterForGPUDesc_.CheckEmitter(commandContext, transformAreaEmitterForGPUs_.size(), transformAreaEmitterForGPUs_.data());

		commandContext.SetComputeUAV(0, emitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(1, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(2, vertexEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(3, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(4, meshEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(5, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(6, transformModelEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(7, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(8, transformAreaEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(9, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());

		commandContext.SetComputeConstantBuffer(10, emitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(11, vertexEmitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(12, meshEmitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(13, transformModelEmitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(14, transformAreaEmitterForGPUDesc_.addCountBuffer->GetGPUVirtualAddress());

		commandContext.Dispatch(uint32_t(sumEmitter), 1, 1);

		commandContext.UAVBarrier(emitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(emitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(vertexEmitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(vertexEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(meshEmitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(meshEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(transformModelEmitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(transformModelEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(transformAreaEmitterForGPUDesc_.defaultCopyBuffer);
		commandContext.UAVBarrier(transformAreaEmitterForGPUDesc_.originalBuffer);
		//commandContext.SetMarker("Marker");
		commandContext.EndEvent();
	}
}

void GPUParticle::AddEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty() ||
		!vertexEmitterForGPUs_.empty() ||
		!meshEmitterForGPUs_.empty() ||
		!transformModelEmitterForGPUs_.empty() ||
		!transformAreaEmitterForGPUs_.empty()) {

		commandContext.BeginEvent(L"AddEmitter");

		emitterForGPUDesc_.AddEmitter(commandContext);
		vertexEmitterForGPUDesc_.AddEmitter(commandContext);
		meshEmitterForGPUDesc_.AddEmitter(commandContext);
		transformModelEmitterForGPUDesc_.AddEmitter(commandContext);
		transformAreaEmitterForGPUDesc_.AddEmitter(commandContext);

		commandContext.SetComputeUAV(0, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(1, emitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(2, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(3, vertexEmitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(4, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(5, meshEmitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(6, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(7, transformModelEmitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(8, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(9, transformAreaEmitterForGPUDesc_.createEmitterBuffer->GetGPUVirtualAddress());


		commandContext.SetComputeUAV(10, emitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(11, vertexEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(12, meshEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(13, transformModelEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(14, transformAreaEmitterForGPUDesc_.defaultCopyBuffer->GetGPUVirtualAddress());
		commandContext.Dispatch(1, 1, 1);
		commandContext.UAVBarrier(emitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(emitterForGPUDesc_.createEmitterBuffer);
		commandContext.UAVBarrier(vertexEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(vertexEmitterForGPUDesc_.createEmitterBuffer);
		commandContext.UAVBarrier(meshEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(meshEmitterForGPUDesc_.createEmitterBuffer);
		commandContext.UAVBarrier(transformModelEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(transformModelEmitterForGPUDesc_.createEmitterBuffer);
		commandContext.UAVBarrier(transformAreaEmitterForGPUDesc_.originalBuffer);
		commandContext.UAVBarrier(transformAreaEmitterForGPUDesc_.createEmitterBuffer);
		emitterForGPUs_.clear();
		vertexEmitterForGPUs_.clear();
		meshEmitterForGPUs_.clear();
		transformModelEmitterForGPUs_.clear();
		emitterForGPUs_.clear();
		transformAreaEmitterForGPUs_.clear();

		commandContext.EndEvent();
	}
}

void GPUParticle::UpdateEmitter(CommandContext& commandContext) {
	// createParticleのリセット
	commandContext.BeginEvent(L"UpdateEmitter");
	commandContext.CopyBufferRegion(createParticleCounterCopySrcBuffer_, 0, resetCounterBuffer_, 0, sizeof(UINT));

	commandContext.TransitionResource(createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createParticleCounterCopySrcBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	emitterForGPUDesc_.UpdateEmitter(commandContext);
	vertexEmitterForGPUDesc_.UpdateEmitter(commandContext);
	meshEmitterForGPUDesc_.UpdateEmitter(commandContext);
	transformModelEmitterForGPUDesc_.UpdateEmitter(commandContext);
	transformAreaEmitterForGPUDesc_.UpdateEmitter(commandContext);

	commandContext.SetComputeUAV(0, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(1, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(2, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(3, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(4, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());

	commandContext.SetComputeDescriptorTable(5, createParticleBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(6, createParticleCounterCopySrcBuffer_->GetGPUVirtualAddress());

	commandContext.Dispatch(1, 1, 1);
	commandContext.UAVBarrier(createParticleBuffer_);
	commandContext.UAVBarrier(emitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(vertexEmitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(meshEmitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(transformModelEmitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(transformAreaEmitterForGPUDesc_.originalBuffer);
	commandContext.UAVBarrier(createParticleCounterCopySrcBuffer_);
	// x
	// 合計何個のパーティクルを生成するのか
	commandContext.CopyBufferRegion(spawnArgumentBuffer_, 0, createParticleCounterCopySrcBuffer_, 0, sizeof(UINT));
	// y
	// エミッターの数
	commandContext.CopyBufferRegion(spawnArgumentBuffer_, sizeof(UINT), createParticleBuffer_, createParticleBuffer_.GetCounterOffset(), sizeof(UINT));
	commandContext.EndEvent();
}

void GPUParticle::Spawn(CommandContext& commandContext, const UploadBuffer& random) {
	commandContext.BeginEvent(L"Spawn");
	// あと何個生成できるかコピー
	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(spawnArgumentBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	emitterForGPUDesc_.Spawn(commandContext);
	vertexEmitterForGPUDesc_.Spawn(commandContext);
	meshEmitterForGPUDesc_.Spawn(commandContext);
	transformModelEmitterForGPUDesc_.Spawn(commandContext);
	transformAreaEmitterForGPUDesc_.Spawn(commandContext);

	commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, createParticleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(3, originalCounterBuffer_->GetGPUVirtualAddress());

	commandContext.SetComputeDescriptorTable(4, trailsIndexBuffers_.GetUAVHandle());
	commandContext.SetComputeUAV(5, trailsDataBuffers_->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(6, trailsHeadBuffers_->GetGPUVirtualAddress());

	commandContext.SetComputeShaderResource(7, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(8, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(9, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(10, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(11, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());

	commandContext.SetComputeDescriptorTable(12, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());
	commandContext.SetComputeDescriptorTable(13, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());

	commandContext.SetComputeConstantBuffer(14, random.GetGPUVirtualAddress());

	commandContext.ExecuteIndirect(
		*spawnCommandSignature_,
		1,
		spawnArgumentBuffer_,
		0,
		nullptr,
		0
	);
	commandContext.UAVBarrier(particleBuffer_);

	//commandContext.CopyBufferRegion(originalCommandCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));
	commandContext.CopyBufferRegion(createParticleBuffer_, createParticleBuffer_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(UINT));
	commandContext.EndEvent();
}


void GPUParticle::ParticleUpdate(const ViewProjection& viewProjection, CommandContext& commandContext) {
	commandContext.BeginEvent(L"ParticleUpdate");
	// リセット
	commandContext.CopyBufferRegion(drawIndexCommandBuffers_, drawIndexCommandBuffers_.GetCounterOffset(), resetCounterBuffer_, 0, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	emitterForGPUDesc_.UpdateParticle(commandContext);
	vertexEmitterForGPUDesc_.UpdateParticle(commandContext);
	meshEmitterForGPUDesc_.UpdateParticle(commandContext);
	transformModelEmitterForGPUDesc_.UpdateParticle(commandContext);
	transformAreaEmitterForGPUDesc_.UpdateParticle(commandContext);


	commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());

	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeDescriptorTable(2, drawIndexCommandBuffers_.GetUAVHandle());

	commandContext.SetComputeShaderResource(3, emitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(4, vertexEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(5, meshEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(6, transformModelEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(7, transformAreaEmitterForGPUDesc_.originalBuffer->GetGPUVirtualAddress());


	commandContext.SetComputeConstantBuffer(8, viewProjection.constBuff_.GetGPUVirtualAddress());

	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(particleBuffer_);
	commandContext.UAVBarrier(drawIndexCommandBuffers_);

	commandContext.EndEvent();
}

void GPUParticle::BulletUpdate(CommandContext& commandContext, const UploadBuffer& random) {
	if (!bullets_.empty()) {
		commandContext.BeginEvent(L"UpdateBullet");
		size_t size = bullets_.size();
		bulletCountBuffer_.Copy(&size, sizeof(UINT));

		bulletsBuffer_.Copy(bullets_.data(), sizeof(GPUParticleShaderStructs::BulletForGPU) * size);

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(bulletsBuffer_.GetBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);

		commandContext.SetComputeDescriptorTable(0, bulletsBuffer_.GetSRV());
		commandContext.SetComputeConstantBuffer(1, bulletCountBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeUAV(2, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(3, random.GetGPUVirtualAddress());
		commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
		commandContext.UAVBarrier(particleBuffer_);
		bullets_.clear();
		commandContext.EndEvent();
	}
}

void GPUParticle::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	commandContext.BeginEvent(L"DrawParticle");
	UINT64 destInstanceCountArgumentOffset = sizeof(GPUParticleShaderStructs::IndirectCommand::SRV) + sizeof(UINT);
	UINT64 srcInstanceCountArgumentOffset = originalCommandBuffer_.GetCounterOffset();

	commandContext.CopyBufferRegion(drawArgumentBuffer_, destInstanceCountArgumentOffset, drawIndexCommandBuffers_, drawIndexCommandBuffers_.GetCounterOffset(), sizeof(UINT));
	// 残りのパーティクルを描画
	//commandContext.CopyBufferRegion(drawIndexCountBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));
	// 出ているパーティクルを描画
	commandContext.CopyBufferRegion(drawIndexCountBuffer_, 0, drawIndexCommandBuffers_, drawIndexCommandBuffers_.GetCounterOffset(), sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(drawArgumentBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext.SetGraphicsConstantBuffer(2, viewProjection.constBuff_.GetGPUVirtualAddress());
	commandContext.SetGraphicsDescriptorTable(3, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());
	commandContext.SetGraphicsDescriptorTable(4, SamplerManager::LinearWrap);
	commandContext.ExecuteIndirect(
		*commandSignature_,
		1,
		drawArgumentBuffer_,
		0,
		nullptr,
		0
	);
	commandContext.EndEvent();
}

void GPUParticle::DrawImGui() {
	ImGui::Begin("GPUParticle");
	ImGui::Text("MaxParticleNum:%d", GPUParticleShaderStructs::MaxParticleNum);
	ImGui::Text("CurrentParticleNum:%d", *static_cast<uint32_t*>(drawIndexCountBuffer_.GetCPUData()));
	ImGui::End();
}

void GPUParticle::CreateMeshParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldMatrix, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.BeginEvent(L"MeshParticle");
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(animation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(model.GetIndexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);

		commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
		commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(3, animation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(4, model.GetIndexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(5, random.GetGPUVirtualAddress());
		ConstBufferDataWorldTransform constBufferDataWorldTransform{};
		constBufferDataWorldTransform.matWorld = worldMatrix;
		constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldMatrix));
		commandContext.SetComputeDynamicConstantBufferView(6, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);
		size_t indexCount = model.GetAllIndexCount();
		commandContext.SetComputeDynamicConstantBufferView(7, sizeof(indexCount), &indexCount);
		commandContext.SetComputeDynamicConstantBufferView(8, sizeof(mesh), &mesh);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), createParticleNum, 1);
		commandContext.UAVBarrier(originalCommandBuffer_);
		commandContext.UAVBarrier(particleBuffer_);
		commandContext.EndEvent();
	}
}

void GPUParticle::CreateMeshParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// auto model = ModelManager::GetInstance()->GetModel(modelHandle);
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.BeginEvent(L"MeshParticle");
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(model.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(model.GetIndexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);

		commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
		commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(3, model.GetVertexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(4, model.GetIndexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(5, random.GetGPUVirtualAddress());
		ConstBufferDataWorldTransform constBufferDataWorldTransform{};
		constBufferDataWorldTransform.matWorld = worldTransform;
		constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
		commandContext.SetComputeDynamicConstantBufferView(6, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);

		size_t indexCount = model.GetAllIndexCount();
		commandContext.SetComputeDynamicConstantBufferView(7, sizeof(indexCount), &indexCount);
		commandContext.SetComputeDynamicConstantBufferView(8, sizeof(mesh), &mesh);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), createParticleNum, 1);
		commandContext.UAVBarrier(originalCommandBuffer_);
		commandContext.UAVBarrier(particleBuffer_);
		commandContext.EndEvent();
	}
}

void GPUParticle::CreateVertexParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	commandContext.BeginEvent(L"VertexParticle");
	auto& model = ModelManager::GetInstance()->GetModel(modelHandle);

	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(animation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, animation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(4, random.GetGPUVirtualAddress());
	ConstBufferDataWorldTransform constBufferDataWorldTransform{};
	constBufferDataWorldTransform.matWorld = worldTransform;
	constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);
	size_t vertexCount = model.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(vertexCount), &vertexCount);
	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(mesh), &mesh);

	size_t numThreadGroups = (vertexCount + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;

	commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
	commandContext.UAVBarrier(particleBuffer_);
	commandContext.EndEvent();
}
void GPUParticle::CreateVertexParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	commandContext.BeginEvent(L"VertexParticle");

	auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
	// あと何個生成できるかコピー
	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, model.GetVertexBuffer().GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(4, random.GetGPUVirtualAddress());
	ConstBufferDataWorldTransform constBufferDataWorldTransform{};
	constBufferDataWorldTransform.matWorld = worldTransform;
	constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));

	size_t vertexCount = model.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);
	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(vertexCount), &vertexCount);
	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(mesh), &mesh);

	size_t numThreadGroups = (vertexCount + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;

	commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
	commandContext.UAVBarrier(particleBuffer_);
	commandContext.EndEvent();
}

void GPUParticle::CreateEdgeParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.BeginEvent(L"EdgeParticle");

		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(animation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(model.GetIndexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);

		commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
		commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(3, animation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(4, model.GetIndexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(5, random.GetGPUVirtualAddress());
		ConstBufferDataWorldTransform constBufferDataWorldTransform{};
		constBufferDataWorldTransform.matWorld = worldTransform;
		constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
		commandContext.SetComputeDynamicConstantBufferView(6, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);

		size_t indexCount = model.GetAllIndexCount();
		commandContext.SetComputeDynamicConstantBufferView(7, sizeof(indexCount), &indexCount);
		commandContext.SetComputeDynamicConstantBufferView(8, sizeof(mesh), &mesh);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		//uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
		commandContext.UAVBarrier(originalCommandBuffer_);
		commandContext.EndEvent();
	}
}

void GPUParticle::CreateEdgeParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterForCPU& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.BeginEvent(L"EdgeParticle");
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);

		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, originalCommandBuffer_.GetCounterOffset(), sizeof(UINT));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(model.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(model.GetIndexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);

		commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
		commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(3, model.GetVertexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(4, model.GetIndexBuffer().GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(5, random.GetGPUVirtualAddress());
		ConstBufferDataWorldTransform constBufferDataWorldTransform{};
		constBufferDataWorldTransform.matWorld = worldTransform;
		constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
		commandContext.SetComputeDynamicConstantBufferView(6, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);

		uint32_t indexCount = model.GetAllIndexCount();
		commandContext.SetComputeDynamicConstantBufferView(7, sizeof(indexCount), &indexCount);
		commandContext.SetComputeDynamicConstantBufferView(8, sizeof(mesh), &mesh);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		//uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
		commandContext.UAVBarrier(originalCommandBuffer_);
		commandContext.UAVBarrier(particleBuffer_);
		commandContext.EndEvent();
	}
}
//
//void GPUParticle::CreateTransformModelParticle(const ModelHandle& startModelHandle, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformModelEmitterForGPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
//	auto modelManager = ModelManager::GetInstance();
//	auto& startModel = modelManager->GetModel(startModelHandle);
//	auto& endModel = modelManager->GetModel(endModelHandle);
//
//	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));
//
//	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(startModel.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
//	//commandContext.TransitionResource(startModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//	commandContext.TransitionResource(endModel.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
//	//commandContext.TransitionResource(endModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//
//
//	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
//	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(3, startModel.GetVertexBuffer().GetGPUVirtualAddress());
//	//commandContext.SetComputeShaderResource(4, startModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(4, endModel.GetVertexBuffer().GetGPUVirtualAddress());
//	//commandContext.SetComputeShaderResource(6, endModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
//	auto startVerticesSize = startModel.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(startVerticesSize), &startVerticesSize);
//	auto endVerticesSize = endModel.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(endVerticesSize), &endVerticesSize);
//	ConstBufferDataWorldTransform startConstBufferDataWorldTransform{};
//	startConstBufferDataWorldTransform.matWorld = startWorldTransform;
//	startConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(startWorldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(ConstBufferDataWorldTransform), &startConstBufferDataWorldTransform);
//	ConstBufferDataWorldTransform endConstBufferDataWorldTransform{};
//	endConstBufferDataWorldTransform.matWorld = endWorldTransform;
//	endConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(endWorldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(8, sizeof(ConstBufferDataWorldTransform), &endConstBufferDataWorldTransform);
//
//	commandContext.SetComputeDynamicConstantBufferView(9, sizeof(transformEmitter), &transformEmitter);
//
//	commandContext.SetComputeConstantBuffer(10, random.GetGPUVirtualAddress());
//
//	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
//	commandContext.UAVBarrier(originalCommandBuffer_);
//}
//
//void GPUParticle::CreateTransformModelParticle(const ModelHandle& startModelHandle, Animation::Animation& startAnimation, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, Animation::Animation& endAnimation, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitterForCPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
//	auto modelManager = ModelManager::GetInstance();
//	auto& startModel = modelManager->GetModel(startModelHandle);
//	auto& endModel = modelManager->GetModel(endModelHandle);
//
//	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));
//
//	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(startAnimation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//	//commandContext.TransitionResource(startModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//	commandContext.TransitionResource(endAnimation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//	//commandContext.TransitionResource(endModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//
//
//	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
//	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(3, startAnimation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
//	//commandContext.SetComputeShaderResource(4, startModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(4, endAnimation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
//	//commandContext.SetComputeShaderResource(6, endModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
//	auto startVerticesSize = startModel.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(startVerticesSize), &startVerticesSize);
//	auto endVerticesSize = endModel.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(endVerticesSize), &endVerticesSize);
//	ConstBufferDataWorldTransform startConstBufferDataWorldTransform{};
//	startConstBufferDataWorldTransform.matWorld = startWorldTransform;
//	startConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(startWorldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(ConstBufferDataWorldTransform), &startConstBufferDataWorldTransform);
//	ConstBufferDataWorldTransform endConstBufferDataWorldTransform{};
//	endConstBufferDataWorldTransform.matWorld = endWorldTransform;
//	endConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(endWorldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(8, sizeof(ConstBufferDataWorldTransform), &endConstBufferDataWorldTransform);
//
//	commandContext.SetComputeDynamicConstantBufferView(9, sizeof(transformEmitter), &transformEmitter);
//
//	commandContext.SetComputeConstantBuffer(10, random.GetGPUVirtualAddress());
//
//	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
//	commandContext.UAVBarrier(originalCommandBuffer_);
//	commandContext.UAVBarrier(particleBuffer_);
//}
//
//void GPUParticle::CreateTransformModelParticle(const ModelHandle& startModelHandle, Animation::Animation& startAnimation, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitterForCPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
//	auto modelManager = ModelManager::GetInstance();
//	auto& startModel = modelManager->GetModel(startModelHandle);
//	auto& endModel = modelManager->GetModel(endModelHandle);
//
//	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));
//
//	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(startAnimation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//	//commandContext.TransitionResource(startModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//	commandContext.TransitionResource(endModel.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
//	//commandContext.TransitionResource(endModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//
//
//	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
//	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(3, startAnimation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
//	//commandContext.SetComputeShaderResource(4, startModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(4, endModel.GetVertexBuffer().GetGPUVirtualAddress());
//	//commandContext.SetComputeShaderResource(6, endModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
//	auto startVerticesSize = startModel.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(startVerticesSize), &startVerticesSize);
//	auto endVerticesSize = endModel.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(endVerticesSize), &endVerticesSize);
//	ConstBufferDataWorldTransform startConstBufferDataWorldTransform{};
//	startConstBufferDataWorldTransform.matWorld = startWorldTransform;
//	startConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(startWorldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(ConstBufferDataWorldTransform), &startConstBufferDataWorldTransform);
//	ConstBufferDataWorldTransform endConstBufferDataWorldTransform{};
//	endConstBufferDataWorldTransform.matWorld = endWorldTransform;
//	endConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(endWorldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(8, sizeof(ConstBufferDataWorldTransform), &endConstBufferDataWorldTransform);
//
//	commandContext.SetComputeDynamicConstantBufferView(9, sizeof(transformEmitter), &transformEmitter);
//
//	commandContext.SetComputeConstantBuffer(10, random.GetGPUVirtualAddress());
//
//	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
//	commandContext.UAVBarrier(originalCommandBuffer_);
//	commandContext.UAVBarrier(particleBuffer_);
//}
//
//void GPUParticle::CreateTransformModelParticle(const ModelHandle& startModelHandle, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, Animation::Animation& endAnimation, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitterForCPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
//	auto modelManager = ModelManager::GetInstance();
//	auto& startModel = modelManager->GetModel(startModelHandle);
//	auto& endModel = modelManager->GetModel(endModelHandle);
//
//	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));
//
//	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(startModel.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
//	//commandContext.TransitionResource(startModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//	commandContext.TransitionResource(endAnimation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//	//commandContext.TransitionResource(endModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
//
//
//	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
//	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(3, startModel.GetVertexBuffer().GetGPUVirtualAddress());
//	//commandContext.SetComputeShaderResource(4, startModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(4, endAnimation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
//	//commandContext.SetComputeShaderResource(6, endModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
//	auto startVerticesSize = startModel.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(startVerticesSize), &startVerticesSize);
//	auto endVerticesSize = endModel.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(endVerticesSize), &endVerticesSize);
//	ConstBufferDataWorldTransform startConstBufferDataWorldTransform{};
//	startConstBufferDataWorldTransform.matWorld = startWorldTransform;
//	startConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(startWorldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(ConstBufferDataWorldTransform), &startConstBufferDataWorldTransform);
//	ConstBufferDataWorldTransform endConstBufferDataWorldTransform{};
//	endConstBufferDataWorldTransform.matWorld = endWorldTransform;
//	endConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(endWorldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(8, sizeof(ConstBufferDataWorldTransform), &endConstBufferDataWorldTransform);
//
//	commandContext.SetComputeDynamicConstantBufferView(9, sizeof(transformEmitter), &transformEmitter);
//
//	commandContext.SetComputeConstantBuffer(10, random.GetGPUVirtualAddress());
//
//	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
//	commandContext.UAVBarrier(originalCommandBuffer_);
//	commandContext.UAVBarrier(particleBuffer_);
//}
//
//void GPUParticle::CreateTransformModelAreaParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::TransformEmitterForCPU& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
//	auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
//
//	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));
//
//	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//	commandContext.TransitionResource(model.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
//
//
//	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
//	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
//	commandContext.SetComputeShaderResource(3, model.GetVertexBuffer().GetGPUVirtualAddress());
//	auto startVerticesSize = model.GetAllVertexCount();
//	commandContext.SetComputeDynamicConstantBufferView(4, sizeof(startVerticesSize), &startVerticesSize);
//	ConstBufferDataWorldTransform constBufferDataWorldTransform{};
//	constBufferDataWorldTransform.matWorld = worldTransform;
//	constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
//	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);
//
//	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(transformEmitter), &transformEmitter);
//
//	commandContext.SetComputeConstantBuffer(7, random.GetGPUVirtualAddress());
//
//	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
//	commandContext.UAVBarrier(originalCommandBuffer_);
//	commandContext.UAVBarrier(particleBuffer_);
//}

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
	particleBuffer_.Create(
		L"particleBuffer",
		UINT64(sizeof(GPUParticleShaderStructs::Particle) * GPUParticleShaderStructs::MaxParticleNum),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	originalCounterBuffer_.Create(L"originalCounterBuffer", sizeof(INT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void GPUParticle::InitializeUpdateParticle() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto& commandContext = RenderManager::GetInstance()->GetCommandContext();

	// 何番目のパーティクルが生きているか積み込みよう
	drawIndexCommandBuffers_.Create(
		L"DrawIndexBuffers",
		GPUParticleShaderStructs::MaxParticleNum * sizeof(uint32_t),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	// UAVView生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = drawIndexCommandBuffers_.GetCounterOffset();
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	drawIndexCommandBuffers_.CreateUAV(uavDesc);

	drawIndexCountBuffer_.Create(L"DrawIndexCountBuffer", sizeof(UINT));
	drawIndexCountBuffer_.ResetBuffer();

	// パーティクルのindexをAppend,Consumeするよう
	originalCommandBuffer_.Create(
		L"originalCommandBuffer",
		sizeof(uint32_t) * GPUParticleShaderStructs::MaxParticleNum,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	// リソースビューを作成した後、UAV を作成
	uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = originalCommandBuffer_.GetCounterOffset();
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	originalCommandBuffer_.CreateUAV(uavDesc);

	// Draw引数用バッファー
	drawArgumentBuffer_.Create(
		L"DrawArgumentBuffer",
		sizeof(GPUParticleShaderStructs::IndirectCommand)
	);
}

void GPUParticle::InitializeSpawnBuffer() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	// spawn引数用バッファー
	spawnArgumentBuffer_.Create(
		L"SpawnArgumentBuffer",
		sizeof(D3D12_DISPATCH_ARGUMENTS)
	);
}

void GPUParticle::InitializeBuffer() {
	enum {
		kParticleIndexBuffer,
		kFieldIndexBuffer,

		kCount,
	};
	CommandContext commandContext;
	commandContext.Create();
	commandContext.Close();
	commandContext.Start(D3D12_COMMAND_LIST_TYPE_DIRECT);
	// スポーンシグネイチャー
	{
		initializeBufferRootSignature_ = std::make_unique<RootSignature>();
		//	createParticle用
		CD3DX12_DESCRIPTOR_RANGE particleIndexBuffer[1]{};
		particleIndexBuffer[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
		CD3DX12_DESCRIPTOR_RANGE fieldIndexBuffer[1]{};
		fieldIndexBuffer[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);
		CD3DX12_ROOT_PARAMETER rootParameters[kCount]{};
		rootParameters[kParticleIndexBuffer].InitAsDescriptorTable(_countof(particleIndexBuffer), particleIndexBuffer);
		rootParameters[kFieldIndexBuffer].InitAsDescriptorTable(_countof(fieldIndexBuffer), fieldIndexBuffer);
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
	commandContext.SetComputeRootSignature(*initializeBufferRootSignature_);
	commandContext.SetPipelineState(*initializeBufferPipelineState_);

	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.SetComputeDescriptorTable(kParticleIndexBuffer, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeDescriptorTable(kFieldIndexBuffer, fieldIndexStockBuffer_.GetUAVHandle());
	commandContext.Dispatch(static_cast<UINT>(ceil(GPUParticleShaderStructs::MaxParticleNum / float(GPUParticleShaderStructs::ComputeThreadBlockSize))), 1, 1);

	// カウンターを初期化
	//UploadBuffer copyDrawIndexCounterBuffer{};
	//UINT counterNum = GPUParticleShaderStructs::MaxParticleNum;
	//copyDrawIndexCounterBuffer.Create(L"copyDrawIndexCounterBuffer", sizeof(counterNum));
	//copyDrawIndexCounterBuffer.Copy(counterNum);

	//commandContext.CopyBufferRegion(originalCommandBuffer_, particleIndexCounterOffset_, copyDrawIndexCounterBuffer, 0, sizeof(UINT));
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
		commandContext.CopyBuffer(fieldAddBuffer_, fieldCPUBuffer_);
		commandContext.CopyBuffer(fieldOriginalBuffer_, fieldCPUBuffer_);
	}

	UploadBuffer drawCopyBuffer{};
	{
		drawCopyBuffer.Create(L"Copy", sizeof(GPUParticleShaderStructs::IndirectCommand));
		GPUParticleShaderStructs::IndirectCommand tmp{};
		tmp.srv.particleSRV = particleBuffer_->GetGPUVirtualAddress();
		tmp.srv.drawIndexSRV = drawIndexCommandBuffers_->GetGPUVirtualAddress();
		tmp.drawIndex.IndexCountPerInstance = UINT(6);
		tmp.drawIndex.InstanceCount = 1;
		tmp.drawIndex.BaseVertexLocation = 0;
		tmp.drawIndex.StartIndexLocation = 0;
		tmp.drawIndex.StartInstanceLocation = 0;
		drawCopyBuffer.Copy(&tmp, sizeof(GPUParticleShaderStructs::IndirectCommand));
		commandContext.CopyBuffer(drawArgumentBuffer_, drawCopyBuffer);

	}
	UploadBuffer spawnCopyBuffer{};
	{

		spawnCopyBuffer.Create(L"Copy", sizeof(D3D12_DISPATCH_ARGUMENTS));
		D3D12_DISPATCH_ARGUMENTS tmp{};
		tmp.ThreadGroupCountX = 1;
		tmp.ThreadGroupCountY = 1;
		tmp.ThreadGroupCountZ = 1;
		spawnCopyBuffer.Copy(&tmp, sizeof(D3D12_DISPATCH_ARGUMENTS));
		commandContext.CopyBuffer(spawnArgumentBuffer_, spawnCopyBuffer);
	}
	commandContext.Close();
	commandContext.Flush();
	commandContext.End();
}

void GPUParticle::InitializeEmitter() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	// resetするよう
	resetCounterBuffer_.Create(L"resetCounterBuffer", sizeof(UINT));
	resetCounterBuffer_.ResetBuffer();


	createParticleBuffer_.Create(L"CreateParticleBuffer", sizeof(GPUParticleShaderStructs::CreateParticle) * GPUParticleShaderStructs::MaxEmitterNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	// UAVを作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxEmitterNum;
	uavDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::CreateParticle);
	uavDesc.Buffer.CounterOffsetInBytes = createParticleBuffer_.GetCounterOffset();
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	createParticleBuffer_.CreateUAV(uavDesc);

	createParticleCounterCopySrcBuffer_.Create(L"CreateParticleCounterCopySrcBuffer", sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
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
	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto graphics = GraphicsCore::GetInstance();
	{
		fieldOriginalBuffer_.Create(L"FieldOriginalBuffer", sizeof(GPUParticleShaderStructs::FieldForGPU) * GPUParticleShaderStructs::MaxFieldNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		fieldCPUBuffer_.Create(L"FieldCPUBuffer", sizeof(GPUParticleShaderStructs::FieldForGPU) * GPUParticleShaderStructs::MaxFieldNum);
		std::array<GPUParticleShaderStructs::FieldForGPU, GPUParticleShaderStructs::MaxFieldNum> reset{};
		fieldCPUBuffer_.Copy(reset.data(), fieldCPUBuffer_.GetBufferSize());
		fieldAddBuffer_.Create(L"FieldAddBuffer", sizeof(GPUParticleShaderStructs::FieldForGPU) * GPUParticleShaderStructs::MaxFieldNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		fieldCounterBuffer_.Create(L"FieldCounterBuffer", sizeof(UINT));
		createFieldNumBuffer_.Create(L"CreateFieldNumBuffer", sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		fieldIndexStockBuffer_.Create(L"FieldIndexStockBuffer", sizeof(UINT) * GPUParticleShaderStructs::MaxFieldNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		fieldIndexBuffer_.Create(L"FieldIndexBuffer", sizeof(UINT) * GPUParticleShaderStructs::MaxFieldNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		// UAVを作成
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxFieldNum;
		uavDesc.Buffer.StructureByteStride = sizeof(UINT);
		uavDesc.Buffer.CounterOffsetInBytes = fieldIndexStockBuffer_.GetCounterOffset();
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		fieldIndexStockBuffer_.CreateUAV(uavDesc);
		fieldIndexBuffer_.CreateUAV(uavDesc);
	}
}

void GPUParticle::InitializeTrails() {
 	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto graphics = GraphicsCore::GetInstance();
	{
		trailsIndexBuffers_.Create(L"trailsIndexBuffers", sizeof(uint32_t) * GPUParticleShaderStructs::MaxTrailsNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		trailsDataBuffers_.Create(L"trailsDataBuffers", sizeof(GPUParticleShaderStructs::TrailsData) * GPUParticleShaderStructs::MaxTrailsNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		trailsHeadBuffers_.Create(L"trailsHeadBuffers_", sizeof(GPUParticleShaderStructs::TrailsHead), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		trailsPositionBuffers_.Create(L"trailsBuffers", sizeof(GPUParticleShaderStructs::TrailsPosition) * GPUParticleShaderStructs::MaxTrailsNum * GPUParticleShaderStructs::TrailsRange, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}
}

void GPUParticle::EmitterDesc::Initialize(CommandContext& commandContext) {
	switch (type) {
	case GPUParticleShaderStructs::EmitterType::kEmitter:
	{
		defaultCopyBuffer.Clear(commandContext);
		originalBuffer.Clear(commandContext);
		size_t size = sizeof(GPUParticleShaderStructs::EmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::EmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(originalBuffer, size, reset.data());
	}

	break;
	case GPUParticleShaderStructs::EmitterType::kVertexEmitter:
	{
		size_t size = sizeof(GPUParticleShaderStructs::VertexEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::VertexEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(originalBuffer, size, reset.data());
	}
	break;
	case GPUParticleShaderStructs::EmitterType::kMeshEmitter:
	{
		size_t size = sizeof(GPUParticleShaderStructs::MeshEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::MeshEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(originalBuffer, size, reset.data());
	}
	break;
	case GPUParticleShaderStructs::EmitterType::kTransformModelEmitter:
	{
		size_t size = sizeof(GPUParticleShaderStructs::TransformModelEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::TransformModelEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(originalBuffer, size, reset.data());
	}
	break;
	case GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter:
	{
		size_t size = sizeof(GPUParticleShaderStructs::TransformAreaEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
		std::vector<GPUParticleShaderStructs::TransformAreaEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
		commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		commandContext.CopyBuffer(originalBuffer, size, reset.data());
	}
	break;
	default:
		break;
	}
}

void GPUParticle::EmitterDesc::Create(const std::wstring& name, const GPUParticleShaderStructs::EmitterType& type) {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();

	{
		size_t addEmitterSize = 0;
		switch (type) {
		case GPUParticleShaderStructs::EmitterType::kEmitter:
		{
			this->type = type;
			addEmitterSize = sizeof(GPUParticleShaderStructs::EmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kVertexEmitter:
		{
			this->type = type;
			addEmitterSize = sizeof(GPUParticleShaderStructs::VertexEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kMeshEmitter:
		{
			this->type = type;
			addEmitterSize = sizeof(GPUParticleShaderStructs::MeshEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kTransformModelEmitter:
		{
			this->type = type;
			addEmitterSize = sizeof(GPUParticleShaderStructs::TransformModelEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter:
		{
			this->type = type;
			addEmitterSize = sizeof(GPUParticleShaderStructs::TransformAreaEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			originalBuffer.Create(name + L"OriginalBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			defaultCopyBuffer.Create(name + L":DefaultCopyBuffer", addEmitterSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		}
		break;
		default:
			break;
		}
		addCountBuffer.Create(name + L":AddCounterBuffer", sizeof(INT));

		createEmitterBuffer.Create(name + L":CreateEmitterBuffer", sizeof(INT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	}
}

size_t GPUParticle::EmitterDesc::CheckEmitter(CommandContext& commandContext, size_t emitterCount, void* data) {
	commandContext.BeginEvent(L"CheckEmitterType");
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
		commandContext.CopyBuffer(defaultCopyBuffer, copySize, data);
	}
	else {
		switch (type) {
		case GPUParticleShaderStructs::EmitterType::kEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::EmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::EmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kVertexEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::VertexEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::VertexEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kMeshEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::MeshEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::MeshEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kTransformModelEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::TransformModelEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::TransformModelEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		}
		break;
		case GPUParticleShaderStructs::EmitterType::kTransformAreaEmitter:
		{
			size_t size = sizeof(GPUParticleShaderStructs::TransformAreaEmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
			std::vector<GPUParticleShaderStructs::TransformAreaEmitterForGPU> reset(GPUParticleShaderStructs::MaxEmitterNum);
			commandContext.CopyBuffer(defaultCopyBuffer, size, reset.data());
		}
		break;
		default:
			break;
		}
	}
	commandContext.CopyBuffer(addCountBuffer, sizeof(uint32_t), &emitterCount);
	commandContext.TransitionResource(defaultCopyBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.EndEvent();
	return emitterCount;
}

void GPUParticle::EmitterDesc::AddEmitter(CommandContext& commandContext) {
	commandContext.BeginEvent(L"AddEmitterType");
	commandContext.CopyBuffer(createEmitterBuffer, addCountBuffer);

	commandContext.TransitionResource(defaultCopyBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createEmitterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.EndEvent();
}

void GPUParticle::EmitterDesc::UpdateEmitter(CommandContext& commandContext) {
	commandContext.TransitionResource(originalBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void GPUParticle::EmitterDesc::UpdateParticle(CommandContext& commandContext) {
	commandContext.TransitionResource(originalBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
}

void GPUParticle::EmitterDesc::Spawn(CommandContext& commandContext) {
	commandContext.TransitionResource(originalBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
}
