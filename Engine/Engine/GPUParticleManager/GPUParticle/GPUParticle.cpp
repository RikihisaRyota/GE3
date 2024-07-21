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
}

GPUParticle::~GPUParticle() {}

void GPUParticle::Initialize() {}

void GPUParticle::CheckField(CommandContext& commandContext) {
	// エミッター追加
	if (!fields_.empty()) {
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
	}
}

void GPUParticle::AddField(CommandContext& commandContext) {
	// フィールド追加
	if (!fields_.empty()) {
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
	}
}

void GPUParticle::UpdateField(CommandContext& commandContext) {

	if (!fields_.empty()) {
		commandContext.CopyBufferRegion(fieldIndexBuffer_, fieldIndexBuffer_.GetCounterOffset(), resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));

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
	}
}

void GPUParticle::CollisionField(CommandContext& commandContext) {
	if (!fields_.empty()) {

		commandContext.TransitionResource(fieldOriginalBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(fieldIndexBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeShaderResource(0, fieldOriginalBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(1, fieldIndexBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(2, particleBuffer_->GetGPUVirtualAddress());
		commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), uint32_t(fields_.size()), 1);
		commandContext.UAVBarrier(particleBuffer_);
		fields_.clear();
	}
}

void GPUParticle::CheckEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty()) {
		size_t emitterCount = emitterForGPUs_.size();
		size_t copySize = sizeof(GPUParticleShaderStructs::EmitterForGPU) * emitterCount;
		emitterCopyUploadBuffer_.ResetBuffer();
		emitterCopyUploadBuffer_.Copy(emitterForGPUs_.data(), copySize);
		commandContext.CopyBuffer(emitterCopyDefaultBuffer_, emitterCopyUploadBuffer_);
		addEmitterCountBuffer_.Copy(&emitterCount, sizeof(UINT));

		commandContext.TransitionResource(emitterCopyDefaultBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, emitterCopyDefaultBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
		commandContext.Dispatch(uint32_t(emitterCount), 1, 1);

		commandContext.UAVBarrier(emitterCopyDefaultBuffer_);
		commandContext.UAVBarrier(emitterForGPUBuffer_);
	}
}

void GPUParticle::AddEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty()) {
		commandContext.CopyBufferRegion(createEmitterBuffer_, 0, addEmitterCountBuffer_, 0, sizeof(UINT));

		commandContext.TransitionResource(emitterCopyDefaultBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(createEmitterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeShaderResource(0, emitterCopyDefaultBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(2, createEmitterBuffer_->GetGPUVirtualAddress());
		commandContext.Dispatch(1, 1, 1);
		commandContext.UAVBarrier(emitterForGPUBuffer_);
		commandContext.UAVBarrier(createEmitterBuffer_);
		emitterForGPUs_.clear();
	}
}

void GPUParticle::EmitterUpdate(CommandContext& commandContext) {
	// createParticleのリセット
	commandContext.CopyBufferRegion(createParticleCounterCopySrcBuffer_, 0, resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));

	commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createParticleCounterCopySrcBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.SetComputeUAV(0, emitterForGPUBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, createParticleBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, createParticleCounterCopySrcBuffer_->GetGPUVirtualAddress());

	commandContext.Dispatch(1, 1, 1);
	commandContext.UAVBarrier(createParticleBuffer_);
	commandContext.UAVBarrier(emitterForGPUBuffer_);
	commandContext.UAVBarrier(createParticleCounterCopySrcBuffer_);
	// x
	commandContext.CopyBufferRegion(spawnArgumentBuffer_, 0, createParticleCounterCopySrcBuffer_, 0, sizeof(UINT));
	// y
	commandContext.CopyBufferRegion(spawnArgumentBuffer_, sizeof(UINT), createParticleBuffer_, emitterIndexCounterOffset_, sizeof(UINT));
}

void GPUParticle::Spawn(CommandContext& commandContext, const UploadBuffer& random) {
	// あと何個生成できるかコピー
	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(spawnArgumentBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);


	commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(2, random.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(3, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(4, createParticleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(5, originalCounterBuffer_->GetGPUVirtualAddress());

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
	commandContext.CopyBuffer(createParticleBuffer_, resetCreateParticleBuffer_);
}


void GPUParticle::ParticleUpdate(const ViewProjection& viewProjection, CommandContext& commandContext) {
	// リセット
	commandContext.CopyBufferRegion(drawIndexCommandBuffers_, particleIndexCounterOffset_, resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);


	commandContext.SetComputeConstantBuffer(0, viewProjection.constBuff_.GetGPUVirtualAddress());
	commandContext.SetComputeUAV(1, particleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(2, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeDescriptorTable(3, drawIndexCommandBuffers_.GetUAVHandle());
	commandContext.SetComputeShaderResource(4, emitterForGPUBuffer_->GetGPUVirtualAddress());

	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(particleBuffer_);
	commandContext.UAVBarrier(drawIndexCommandBuffers_);

	UINT64 destInstanceCountArgumentOffset = sizeof(GPUParticleShaderStructs::IndirectCommand::SRV) + sizeof(UINT);
	UINT64 srcInstanceCountArgumentOffset = particleIndexCounterOffset_;

	commandContext.CopyBufferRegion(drawArgumentBuffer_, destInstanceCountArgumentOffset, drawIndexCommandBuffers_, srcInstanceCountArgumentOffset, sizeof(UINT));
	commandContext.CopyBufferRegion(drawIndexCountBuffer_, 0, drawIndexCommandBuffers_, srcInstanceCountArgumentOffset, sizeof(UINT));
}

void GPUParticle::BulletUpdate(CommandContext& commandContext, const UploadBuffer& random) {
	if (!bullets_.empty()) {
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
	}
}

void GPUParticle::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
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
}

void GPUParticle::DrawImGui() {
	ImGui::Begin("GPUParticle");
	ImGui::Text("MaxParticleNum:%d", GPUParticleShaderStructs::MaxParticleNum);
	ImGui::Text("CurrentParticleNum:%d", *static_cast<uint32_t*>(drawIndexCountBuffer_.GetCPUData()));
	ImGui::End();
}

void GPUParticle::CreateMeshParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldMatrix, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

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
		commandContext.SetComputeDynamicConstantBufferView(8, sizeof(mesh.emitter), &mesh.emitter);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), createParticleNum, 1);
		commandContext.UAVBarrier(originalCommandBuffer_);
		commandContext.UAVBarrier(particleBuffer_);
	}
}

void GPUParticle::CreateMeshParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// auto model = ModelManager::GetInstance()->GetModel(modelHandle);
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

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
		commandContext.SetComputeDynamicConstantBufferView(8, sizeof(mesh.emitter), &mesh.emitter);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), createParticleNum, 1);
		commandContext.UAVBarrier(originalCommandBuffer_);
		commandContext.UAVBarrier(particleBuffer_);
	}
}

void GPUParticle::CreateVertexParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {

	auto& model = ModelManager::GetInstance()->GetModel(modelHandle);

	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

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
	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(mesh.emitter), &mesh.emitter);

	size_t numThreadGroups = (vertexCount + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;

	commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
	commandContext.UAVBarrier(particleBuffer_);
}
void GPUParticle::CreateVertexParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::VertexEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
	// あと何個生成できるかコピー
	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

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
	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(mesh.emitter), &mesh.emitter);

	size_t numThreadGroups = (vertexCount + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;

	commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
	commandContext.UAVBarrier(particleBuffer_);
}

void GPUParticle::CreateEdgeParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);
		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

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
		commandContext.SetComputeDynamicConstantBufferView(8, sizeof(mesh.emitter), &mesh.emitter);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		//uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
		commandContext.UAVBarrier(originalCommandBuffer_);
	}
}

void GPUParticle::CreateEdgeParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		auto& model = ModelManager::GetInstance()->GetModel(modelHandle);

		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

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
		commandContext.SetComputeDynamicConstantBufferView(8, sizeof(mesh.emitter), &mesh.emitter);

		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		//uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
		commandContext.UAVBarrier(originalCommandBuffer_);
		commandContext.UAVBarrier(particleBuffer_);
	}
}

void GPUParticle::CreateTransformModelParticle(const ModelHandle& startModelHandle, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
	auto modelManager = ModelManager::GetInstance();
	auto& startModel = modelManager->GetModel(startModelHandle);
	auto& endModel = modelManager->GetModel(endModelHandle);

	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(startModel.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(startModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(endModel.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(endModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);


	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, startModel.GetVertexBuffer().GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(4, startModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(4, endModel.GetVertexBuffer().GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(6, endModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
	auto startVerticesSize = startModel.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(startVerticesSize), &startVerticesSize);
	auto endVerticesSize = endModel.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(endVerticesSize), &endVerticesSize);
	ConstBufferDataWorldTransform startConstBufferDataWorldTransform{};
	startConstBufferDataWorldTransform.matWorld = startWorldTransform;
	startConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(startWorldTransform));
	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(ConstBufferDataWorldTransform), &startConstBufferDataWorldTransform);
	ConstBufferDataWorldTransform endConstBufferDataWorldTransform{};
	endConstBufferDataWorldTransform.matWorld = endWorldTransform;
	endConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(endWorldTransform));
	commandContext.SetComputeDynamicConstantBufferView(8, sizeof(ConstBufferDataWorldTransform), &endConstBufferDataWorldTransform);

	commandContext.SetComputeDynamicConstantBufferView(9, sizeof(transformEmitter), &transformEmitter);

	commandContext.SetComputeConstantBuffer(10, random.GetGPUVirtualAddress());

	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
}

void GPUParticle::CreateTransformModelParticle(const ModelHandle& startModelHandle, Animation::Animation& startAnimation, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, Animation::Animation& endAnimation, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
	auto modelManager = ModelManager::GetInstance();
	auto& startModel = modelManager->GetModel(startModelHandle);
	auto& endModel = modelManager->GetModel(endModelHandle);

	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(startAnimation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(startModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(endAnimation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(endModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);


	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, startAnimation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(4, startModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(4, endAnimation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(6, endModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
	auto startVerticesSize = startModel.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(startVerticesSize), &startVerticesSize);
	auto endVerticesSize = endModel.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(endVerticesSize), &endVerticesSize);
	ConstBufferDataWorldTransform startConstBufferDataWorldTransform{};
	startConstBufferDataWorldTransform.matWorld = startWorldTransform;
	startConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(startWorldTransform));
	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(ConstBufferDataWorldTransform), &startConstBufferDataWorldTransform);
	ConstBufferDataWorldTransform endConstBufferDataWorldTransform{};
	endConstBufferDataWorldTransform.matWorld = endWorldTransform;
	endConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(endWorldTransform));
	commandContext.SetComputeDynamicConstantBufferView(8, sizeof(ConstBufferDataWorldTransform), &endConstBufferDataWorldTransform);

	commandContext.SetComputeDynamicConstantBufferView(9, sizeof(transformEmitter), &transformEmitter);

	commandContext.SetComputeConstantBuffer(10, random.GetGPUVirtualAddress());

	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
	commandContext.UAVBarrier(particleBuffer_);
}

void GPUParticle::CreateTransformModelParticle(const ModelHandle& startModelHandle, Animation::Animation& startAnimation, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
	auto modelManager = ModelManager::GetInstance();
	auto& startModel = modelManager->GetModel(startModelHandle);
	auto& endModel = modelManager->GetModel(endModelHandle);

	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(startAnimation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(startModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(endModel.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(endModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);


	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, startAnimation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(4, startModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(4, endModel.GetVertexBuffer().GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(6, endModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
	auto startVerticesSize = startModel.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(startVerticesSize), &startVerticesSize);
	auto endVerticesSize = endModel.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(endVerticesSize), &endVerticesSize);
	ConstBufferDataWorldTransform startConstBufferDataWorldTransform{};
	startConstBufferDataWorldTransform.matWorld = startWorldTransform;
	startConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(startWorldTransform));
	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(ConstBufferDataWorldTransform), &startConstBufferDataWorldTransform);
	ConstBufferDataWorldTransform endConstBufferDataWorldTransform{};
	endConstBufferDataWorldTransform.matWorld = endWorldTransform;
	endConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(endWorldTransform));
	commandContext.SetComputeDynamicConstantBufferView(8, sizeof(ConstBufferDataWorldTransform), &endConstBufferDataWorldTransform);

	commandContext.SetComputeDynamicConstantBufferView(9, sizeof(transformEmitter), &transformEmitter);

	commandContext.SetComputeConstantBuffer(10, random.GetGPUVirtualAddress());

	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
	commandContext.UAVBarrier(particleBuffer_);
}

void GPUParticle::CreateTransformModelParticle(const ModelHandle& startModelHandle, const Matrix4x4& startWorldTransform, const ModelHandle& endModelHandle, Animation::Animation& endAnimation, const Matrix4x4& endWorldTransform, const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
	auto modelManager = ModelManager::GetInstance();
	auto& startModel = modelManager->GetModel(startModelHandle);
	auto& endModel = modelManager->GetModel(endModelHandle);

	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(startModel.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(startModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(endAnimation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	//commandContext.TransitionResource(endModel.GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);


	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, startModel.GetVertexBuffer().GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(4, startModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(4, endAnimation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
	//commandContext.SetComputeShaderResource(6, endModel.GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
	auto startVerticesSize = startModel.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(startVerticesSize), &startVerticesSize);
	auto endVerticesSize = endModel.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(endVerticesSize), &endVerticesSize);
	ConstBufferDataWorldTransform startConstBufferDataWorldTransform{};
	startConstBufferDataWorldTransform.matWorld = startWorldTransform;
	startConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(startWorldTransform));
	commandContext.SetComputeDynamicConstantBufferView(7, sizeof(ConstBufferDataWorldTransform), &startConstBufferDataWorldTransform);
	ConstBufferDataWorldTransform endConstBufferDataWorldTransform{};
	endConstBufferDataWorldTransform.matWorld = endWorldTransform;
	endConstBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(endWorldTransform));
	commandContext.SetComputeDynamicConstantBufferView(8, sizeof(ConstBufferDataWorldTransform), &endConstBufferDataWorldTransform);

	commandContext.SetComputeDynamicConstantBufferView(9, sizeof(transformEmitter), &transformEmitter);

	commandContext.SetComputeConstantBuffer(10, random.GetGPUVirtualAddress());

	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
	commandContext.UAVBarrier(particleBuffer_);
}

void GPUParticle::CreateTransformModelAreaParticle(const ModelHandle& modelHandle, const Matrix4x4& worldTransform, const GPUParticleShaderStructs::TransformEmitter& transformEmitter, const UploadBuffer& random, CommandContext& commandContext) {
	auto& model = ModelManager::GetInstance()->GetModel(modelHandle);

	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCounterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(model.GetVertexBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);


	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandBuffer_.GetUAVHandle());
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, model.GetVertexBuffer().GetGPUVirtualAddress());
	auto startVerticesSize = model.GetAllVertexCount();
	commandContext.SetComputeDynamicConstantBufferView(4, sizeof(startVerticesSize), &startVerticesSize);
	ConstBufferDataWorldTransform constBufferDataWorldTransform{};
	constBufferDataWorldTransform.matWorld = worldTransform;
	constBufferDataWorldTransform.inverseMatWorld = Transpose(Inverse(worldTransform));
	commandContext.SetComputeDynamicConstantBufferView(5, sizeof(ConstBufferDataWorldTransform), &constBufferDataWorldTransform);

	commandContext.SetComputeDynamicConstantBufferView(6, sizeof(transformEmitter), &transformEmitter);

	commandContext.SetComputeConstantBuffer(7, random.GetGPUVirtualAddress());

	commandContext.Dispatch(static_cast<UINT>(ceil((GPUParticleShaderStructs::MaxParticleNum / GPUParticleShaderStructs::MaxProcessNum) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	commandContext.UAVBarrier(originalCommandBuffer_);
	commandContext.UAVBarrier(particleBuffer_);
}

void GPUParticle::SetField(const GPUParticleShaderStructs::FieldForCPU& fieldForCPU) {
	GPUParticleShaderStructs::FieldForGPU fieldForGPU{};
	fieldForGPU.field = fieldForCPU.field;
	fieldForGPU.fieldArea = fieldForCPU.fieldArea;
	fieldForGPU.frequency = fieldForCPU.frequency;
	fieldForGPU.collisionInfo.mask = CollisionAttribute::ParticleField;
	fieldForGPU.isAlive = fieldForCPU.isAlive;
	fieldForGPU.fieldCount = fieldForCPU.fieldCount;
	fields_.emplace_back(fieldForGPU);
}

void GPUParticle::SetEmitter(const GPUParticleShaderStructs::EmitterForCPU& emitterForCPU, const Matrix4x4& parent) {
	GPUParticleShaderStructs::EmitterForGPU emitterForGPU{};
	emitterForGPU.parent = emitterForCPU.parent;
	emitterForGPU.parent.emitterType = GPUParticleShaderStructs::EmitterType::kEmitter;
	emitterForGPU.parent.worldMatrix = parent;
	emitterForGPU.emitterArea = emitterForCPU.emitterArea;
	emitterForGPU.scale = emitterForCPU.scale;
	emitterForGPU.rotate.initializeAngle.min = DegToRad(emitterForCPU.rotate.initializeAngle.min);
	emitterForGPU.rotate.initializeAngle.max = DegToRad(emitterForCPU.rotate.initializeAngle.max);
	emitterForGPU.rotate.rotateSpeed.min = DegToRad(emitterForCPU.rotate.rotateSpeed.min);
	emitterForGPU.rotate.rotateSpeed.max = DegToRad(emitterForCPU.rotate.rotateSpeed.max);
	emitterForGPU.velocity = emitterForCPU.velocity;
	emitterForGPU.color = emitterForCPU.color;
	emitterForGPU.frequency = emitterForCPU.frequency;
	emitterForGPU.time.particleTime = emitterForCPU.frequency.interval;
	emitterForGPU.time.emitterTime = 0;
	emitterForGPU.particleLifeSpan = emitterForCPU.particleLifeSpan;
	emitterForGPU.textureIndex = emitterForCPU.textureIndex;
	emitterForGPU.createParticleNum = emitterForCPU.createParticleNum;
	emitterForGPU.isAlive = emitterForCPU.isAlive;
	emitterForGPU.emitterCount = emitterForCPU.emitterCount;
	emitterForGPU.collisionInfo = emitterForCPU.collisionInfo;
	emitterForGPUs_.emplace_back(emitterForGPU);
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
}

void GPUParticle::InitializeUpdateParticle() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto& commandContext = RenderManager::GetInstance()->GetCommandContext();

	particleIndexSize_ = GPUParticleShaderStructs::MaxParticleNum * sizeof(uint32_t);
	particleIndexCounterOffset_ = AlignForUavCounter(particleIndexSize_);

	// 何番目のパーティクルが生きているか積み込みよう
	drawIndexCommandBuffers_.Create(
		L"DrawIndexBuffers",
		particleIndexCounterOffset_,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	// UAVView生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = particleIndexCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	drawIndexCommandBuffers_.CreateUAV(uavDesc);
	// リセット用
	UINT resetValue = 0;
	resetAppendDrawIndexBufferCounterReset_.Create(L"ResetAppendDrawIndexBufferCounterReset", sizeof(resetValue));
	resetAppendDrawIndexBufferCounterReset_.Copy(resetValue);

	drawIndexCountBuffer_.Create(L"DrawIndexCountBuffer", sizeof(UINT));
	drawIndexCountBuffer_.ResetBuffer();

	//originalCommandCounterBuffer_.Create(L"originalCommandCounterBuffer", sizeof(resetValue));
	//originalCommandCounterBuffer_.Copy(resetValue);

	// パーティクルのindexをAppend,Consumeするよう
	originalCommandBuffer_.Create(
		L"originalCommandBuffer",
		particleIndexCounterOffset_,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	// リソースビューを作成した後、UAV を作成
	uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = particleIndexCounterOffset_;
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
	originalCounterBuffer_.Create(L"originalCounterBuffer", sizeof(INT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
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
		commandContext.CopyBuffer(emitterCopyDefaultBuffer_, emitterCopyUploadBuffer_);
		commandContext.CopyBuffer(emitterForGPUBuffer_, emitterCopyUploadBuffer_);
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

	emitterIndexSize_ = GPUParticleShaderStructs::MaxEmitterNum * sizeof(GPUParticleShaderStructs::CreateParticle);
	emitterIndexCounterOffset_ = AlignForUavCounter(emitterIndexSize_);

	// エミッターバッファー
	emitterForGPUBuffer_.Create(
		L"EmitterForGPUBuffer",
		(sizeof(GPUParticleShaderStructs::EmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	// EmitterのIndexをAppend,Consumeするよう
	resetCreateParticleBuffer_.Create(
		L"resetCreateParticleBuffer",
		emitterIndexCounterOffset_
	);

	std::memset(resetCreateParticleBuffer_.GetCPUData(), 0, resetCreateParticleBuffer_.GetBufferSize());

	createParticleBuffer_.Create(
		L"CreateParticleBuffer",
		emitterIndexCounterOffset_,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	// UAVを作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxEmitterNum;
	uavDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::CreateParticle);
	uavDesc.Buffer.CounterOffsetInBytes = emitterIndexCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	createParticleBuffer_.CreateUAV(uavDesc);

	createParticleCounterCopySrcBuffer_.Create(
		L"CreateParticleCounterCopySrcBuffer",
		sizeof(UINT),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
}

void GPUParticle::InitializeAddEmitter() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();


	addEmitterSize_ = sizeof(GPUParticleShaderStructs::EmitterForGPU) * GPUParticleShaderStructs::MaxEmitterNum;
	addEmitterCounterOffset_ = AlignForUavCounter(addEmitterSize_);

	emitterCopyUploadBuffer_.Create(L"EmitterCopyUploadBuffer", addEmitterSize_);
	emitterCopyDefaultBuffer_.Create(L"EmitterCopyDefaultBuffer", addEmitterSize_, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	std::array< GPUParticleShaderStructs::EmitterForGPU, GPUParticleShaderStructs::MaxEmitterNum> reset{};
	emitterCopyUploadBuffer_.Copy(reset.data(), addEmitterSize_);


	addEmitterCountBuffer_.Create(L"AddEmitterCounterBuffer", sizeof(UINT));
	addEmitterCountBuffer_.ResetBuffer();

	createEmitterBuffer_.Create(L"createEmitterBuffer_",
		sizeof(INT),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
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
