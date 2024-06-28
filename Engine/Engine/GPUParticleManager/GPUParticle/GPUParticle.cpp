#include "GPUParticle.h"

#include <array>

#include <d3dx12.h>

#include "imgui.h"

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

	InitializeBuffer();
}

GPUParticle::~GPUParticle() {}

void GPUParticle::Initialize() {}

void GPUParticle::CheckEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty()) {
		size_t emitterCount = emitterForGPUs_.size();
		size_t copySize = sizeof(GPUParticleShaderStructs::EmitterForGPU) * emitterCount;
		emitterCopyUploadBuffer_.Copy(emitterForGPUs_.data(), copySize);
		commandContext.CopyBuffer(emitterCopyDefaultBuffer_, emitterCopyUploadBuffer_);
		addEmitterCountBuffer_.Copy(&emitterCount, sizeof(UINT));

		commandContext.TransitionResource(emitterCopyDefaultBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, emitterCopyDefaultBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
		commandContext.Dispatch(uint32_t(emitterCount), 1, 1);

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
	commandContext.SetComputeDescriptorTable(1, createParticleUAVHandle_);
	commandContext.SetComputeUAV(2, createParticleCounterCopySrcBuffer_->GetGPUVirtualAddress());

	commandContext.Dispatch(1, 1, 1);
	commandContext.CopyBufferRegion(spawnArgumentBuffer_, 0, createParticleCounterCopySrcBuffer_, 0, sizeof(UINT));
}

void GPUParticle::Spawn(CommandContext& commandContext, const UploadBuffer& random) {
	// あと何個生成できるかコピー
	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.TransitionResource(spawnArgumentBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(2, random.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(3, originalCommandUAVHandle_);
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


	commandContext.SetComputeConstantBuffer(0, viewProjection.constBuff_.GetGPUVirtualAddress());
	commandContext.SetComputeUAV(1, particleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(2, originalCommandUAVHandle_);
	commandContext.SetComputeDescriptorTable(3, drawIndexCommandUAVHandle_);

	commandContext.Dispatch(static_cast<UINT>(ceil(GPUParticleShaderStructs::MaxParticleNum / float(GPUParticleShaderStructs::ComputeThreadBlockSize))), 1, 1);
	commandContext.UAVBarrier(particleBuffer_);
	commandContext.UAVBarrier(drawIndexCommandBuffers_);

	UINT64 destInstanceCountArgumentOffset = sizeof(GPUParticleShaderStructs::IndirectCommand::SRV) + sizeof(UINT);
	UINT64 srcInstanceCountArgumentOffset = particleIndexCounterOffset_;

	commandContext.CopyBufferRegion(drawArgumentBuffer_, destInstanceCountArgumentOffset, drawIndexCommandBuffers_, srcInstanceCountArgumentOffset, sizeof(UINT));
	commandContext.CopyBufferRegion(drawIndexCountBuffer_, 0, drawIndexCommandBuffers_, srcInstanceCountArgumentOffset, sizeof(UINT));
}

void GPUParticle::BulletUpdate(CommandContext& commandContext) {
	if (*static_cast<uint32_t*>(bulletCountBuffer_.GetCPUData()) != 0) {
		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(bulletsBuffer_.GetBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ);

		commandContext.SetComputeDescriptorTable(0, bulletsBuffer_.GetSRV());
		commandContext.SetComputeConstantBuffer(1, bulletCountBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeUAV(2, particleBuffer_.GetGPUVirtualAddress());
		commandContext.Dispatch(static_cast<UINT>(ceil(GPUParticleShaderStructs::MaxParticleNum / float(GPUParticleShaderStructs::ComputeThreadBlockSize))), 1, 1);
		commandContext.UAVBarrier(particleBuffer_);

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

void GPUParticle::CreateMeshParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const WorldTransform& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

		mesh.buffer.Copy(&mesh.emitter, sizeof(mesh.emitter));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(animation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

		commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandUAVHandle_);
		commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(3, animation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(4, ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(5, random.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(6, worldTransform.constBuff.get()->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(7, ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->indexCountBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(8, mesh.buffer.GetGPUVirtualAddress());

		size_t indexCount = ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->meshes->indexCount;
		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), createParticleNum, 1);
	}
}

void GPUParticle::CreateMeshParticle(const ModelHandle& modelHandle, const WorldTransform& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// auto model = ModelManager::GetInstance()->GetModel(modelHandle);
	// あと何個生成できるかコピー
	if (mesh.numCreate != 0) {
		commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

		mesh.buffer.Copy(&mesh.emitter, sizeof(mesh.emitter));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

		commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandUAVHandle_);
		commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(3, ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->vertexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(4, ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->indexBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(5, random.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(6, worldTransform.constBuff.get()->GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(7, ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->indexCountBuffer.GetGPUVirtualAddress());
		commandContext.SetComputeConstantBuffer(8, mesh.buffer.GetGPUVirtualAddress());

		size_t indexCount = ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->meshes->indexCount;
		size_t numTriangles = indexCount / 3;
		size_t numThreadGroups = (numTriangles + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;
		uint32_t createParticleNum = mesh.numCreate;
		commandContext.Dispatch(UINT(numThreadGroups), createParticleNum, 1);
	}
}

void GPUParticle::CreateVertexParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const WorldTransform& worldTransform, const GPUParticleShaderStructs::VertexEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	mesh.buffer.Copy(&mesh.emitter, sizeof(mesh.emitter));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(animation.skinCluster.vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandUAVHandle_);
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, animation.skinCluster.vertexBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(4, random.GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(5, worldTransform.constBuff.get()->GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(6, ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->vertexCountBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(7, mesh.buffer.GetGPUVirtualAddress());

	size_t vertexCount = ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->vertices.size();
	size_t numThreadGroups = (vertexCount + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;

	commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
}

void GPUParticle::CreateVertexParticle(const ModelHandle& modelHandle, const WorldTransform& worldTransform, const GPUParticleShaderStructs::VertexEmitterDesc& mesh, const UploadBuffer& random, CommandContext& commandContext) {
	// あと何個生成できるかコピー
	commandContext.CopyBufferRegion(originalCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));

	mesh.buffer.Copy(&mesh.emitter, sizeof(mesh.emitter));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->vertexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->indexBuffer, D3D12_RESOURCE_STATE_GENERIC_READ);

	commandContext.SetComputeUAV(0, particleBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandUAVHandle_);
	commandContext.SetComputeUAV(2, originalCounterBuffer_.GetGPUVirtualAddress());
	commandContext.SetComputeShaderResource(3, ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->vertexBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(4, random.GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(5, worldTransform.constBuff.get()->GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(6, ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->vertexCountBuffer.GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(7, mesh.buffer.GetGPUVirtualAddress());

	size_t vertexCount = ModelManager::GetInstance()->GetModel(modelHandle).GetMeshData().at(0)->vertices.size();
	size_t numThreadGroups = (vertexCount + GPUParticleShaderStructs::ComputeThreadBlockSize - 1) / GPUParticleShaderStructs::ComputeThreadBlockSize;

	commandContext.Dispatch(UINT(numThreadGroups), 1, 1);
}

void GPUParticle::Create(const GPUParticleShaderStructs::EmitterForCPU& emitterForCPU) {
	GPUParticleShaderStructs::EmitterForGPU emitterForGPU{};
	emitterForGPU.emitterArea = emitterForCPU.emitterArea;
	emitterForGPU.scale = emitterForCPU.scale;
	emitterForGPU.rotate = emitterForCPU.rotate;
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
	emitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetEmitter(const GPUParticleShaderStructs::EmitterForCPU& emitterForCPU) {
	GPUParticleShaderStructs::EmitterForGPU emitterForGPU{};
	emitterForGPU.emitterArea = emitterForCPU.emitterArea;
	emitterForGPU.scale = emitterForCPU.scale;
	emitterForGPU.rotate = emitterForCPU.rotate;
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
	emitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetBullets(const std::vector<GPUParticleShaderStructs::BulletForGPU>& bullets) {
	size_t size = bullets.size();

	bulletCountBuffer_.Copy(&size, sizeof(UINT));

	bulletsBuffer_.Copy(bullets.data(), sizeof(GPUParticleShaderStructs::BulletForGPU) * size);
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
		particleIndexCounterOffset_ + sizeof(UINT),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	drawIndexCommandUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// UAVView生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = particleIndexCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	device->CreateUnorderedAccessView(
		drawIndexCommandBuffers_,
		drawIndexCommandBuffers_,
		&uavDesc,
		drawIndexCommandUAVHandle_);

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
		particleIndexCounterOffset_ + sizeof(UINT),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	originalCommandUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// リソースビューを作成した後、UAV を作成
	uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = particleIndexCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	device->CreateUnorderedAccessView(
		originalCommandBuffer_,
		originalCommandBuffer_,
		&uavDesc,
		originalCommandUAVHandle_);

	// Draw引数用バッファー
	drawArgumentBuffer_.Create(
		L"DrawArgumentBuffer",
		sizeof(GPUParticleShaderStructs::IndirectCommand)
	);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = 1;
	srvDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::IndirectCommand);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	drawArgumentHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(
		drawArgumentBuffer_,
		&srvDesc,
		drawArgumentHandle_
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
		kOriginalBuffer,
		kCount,
	};
	RootSignature initializeBufferRootSignature{};
	PipelineState initializeBufferPipelineState{};
	auto& commandContext = RenderManager::GetInstance()->GetCommandContext();
	// スポーンシグネイチャー
	{
		//	createParticle用
		CD3DX12_DESCRIPTOR_RANGE counterRange[1]{};
		counterRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
		CD3DX12_ROOT_PARAMETER rootParameters[kCount]{};
		rootParameters[kOriginalBuffer].InitAsDescriptorTable(_countof(counterRange), counterRange);
		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		initializeBufferRootSignature.Create(L"InitializeBufferRootSignature", desc);
	}
	// スポーンパイプライン
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = initializeBufferRootSignature;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/InitializeGPUParticle.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		initializeBufferPipelineState.Create(L"InitializeBufferCPSO", desc);
	}
	commandContext.SetComputeRootSignature(initializeBufferRootSignature);
	commandContext.SetPipelineState(initializeBufferPipelineState);

	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.SetComputeDescriptorTable(kOriginalBuffer, originalCommandUAVHandle_);
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
	// コピー
	commandContext.Close();
	CommandQueue& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue();
	commandQueue.Execute(commandContext);
	commandQueue.Signal();
	commandQueue.WaitForGPU();
	commandContext.Reset();
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
		emitterIndexCounterOffset_ + sizeof(UINT)
	);

	std::memset(resetCreateParticleBuffer_.GetCPUData(), 0, resetCreateParticleBuffer_.GetBufferSize());

	createParticleBuffer_.Create(
		L"CreateParticleBuffer",
		emitterIndexCounterOffset_ + sizeof(UINT),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	createParticleUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// UAVを作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxEmitterNum;
	uavDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::CreateParticle);
	uavDesc.Buffer.CounterOffsetInBytes = emitterIndexCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	device->CreateUnorderedAccessView(
		createParticleBuffer_,
		createParticleBuffer_,
		&uavDesc,
		createParticleUAVHandle_);

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
