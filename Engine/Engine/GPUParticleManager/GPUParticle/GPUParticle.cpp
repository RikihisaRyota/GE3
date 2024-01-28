#include "GPUParticle.h"

#include <d3dx12.h>

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
#include "Engine/Model/ModelManager.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Input/Input.h"

//#define STR(x) (#x)
//
//#define Serialize(x) f(STR(x), &x)
//#define DeSerialize(x) f(STR(x), &x)
//
//
//int main() {
//	int power = 0;
//
//	const char name[] = STR(power);
//
//	Serialize(power);
//	DeSerialize(power);
//}

GPUParticle::GPUParticle() {}

GPUParticle::~GPUParticle() {}

void GPUParticle::Initialize() {}

void GPUParticle::Spawn(CommandContext& commandContext) {
	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(emitterCounterBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);

	commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeUAV(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(2, emitterCounterBuffer_->GetGPUVirtualAddress());
	commandContext.Dispatch(1, 1, 1);
}

void GPUParticle::Update(CommandContext& commandContext) {

	// リセット
	commandContext.CopyBufferRegion(appendDrawIndexBuffers_, commandSizePerFrame_, resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));

	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(appendDrawIndexBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	//commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);

	commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, originalCommandSRVHandle_);
	commandContext.SetComputeDescriptorTable(2, appendDrawIndexUAVHandle_);
	//commandContext.SetComputeConstantBuffer(3, emitterForGPUBuffer_->GetGPUVirtualAddress());

	commandContext.Dispatch(emitterForGPU_->maxParticleNum, 1, 1);

	UINT64 destInstanceCountArgumentOffset = sizeof(IndirectCommand::SRV) + sizeof(UINT);
	UINT64 srcInstanceCountArgumentOffset = appendBufferCounterOffset_;

	commandContext.CopyBufferRegion(drawArgumentBuffer_, destInstanceCountArgumentOffset, appendDrawIndexBuffers_, srcInstanceCountArgumentOffset, sizeof(UINT));

}

void GPUParticle::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(appendDrawIndexBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.TransitionResource(drawArgumentBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext.SetGraphicsConstantBuffer(2, viewProjection.constBuff_.GetGPUVirtualAddress());
	commandContext.SetGraphicsDescriptorTable(3, TextureManager::GetInstance()->GetTexture(texture_).GetSRV());
	commandContext.SetGraphicsDescriptorTable(4, SamplerManager::LinearWrap);
	commandContext.ExecuteIndirect(
		commandSignature_,
		1,
		drawArgumentBuffer_,
		0,
		nullptr,
		0
	);
}

void GPUParticle::Create(EmitterForGPU emitterForGPU, TextureHandle textureHandle) {
	texture_ = textureHandle;
	InitializeEmitter(emitterForGPU);
	InitializeParticleBuffer();
	InitializeUpdateParticle();

}

void GPUParticle::InitializeParticleBuffer() {
	// ParticleBuffer
	auto device = GraphicsCore::GetInstance()->GetDevice();
	{
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(
			UINT64(sizeof(Particle) * emitterForGPU_->maxParticleNum),
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(particleBuffer_.GetAddressOf())
		);
		particleBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
		particleBuffer_->SetName(L"particleBuffer");
	}
}

void GPUParticle::InitializeUpdateParticle() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto& commandContext = RenderManager::GetInstance()->GetCommandContext();

	commandSizePerFrame_ = emitterForGPU_->maxParticleNum * sizeof(uint32_t);
	appendBufferCounterOffset_ = AlignForUavCounter(commandSizePerFrame_);

	// 計算結果を積み込むよう
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(appendBufferCounterOffset_ + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(appendDrawIndexBuffers_.GetAddressOf())
	);
	appendDrawIndexBuffers_.SetState(D3D12_RESOURCE_STATE_COMMON);
	appendDrawIndexBuffers_->SetName(L"AppendDrawIndexBuffers");
	appendDrawIndexUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// UAVView生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = emitterForGPU_->maxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = appendBufferCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	device->CreateUnorderedAccessView(
		appendDrawIndexBuffers_,
		appendDrawIndexBuffers_,
		&uavDesc,
		appendDrawIndexUAVHandle_);

	// リセット用

	desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT));
	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(resetAppendDrawIndexBufferCounterReset_.GetAddressOf())
	);
	resetAppendDrawIndexBufferCounterReset_.SetState(D3D12_RESOURCE_STATE_COMMON);
	resetAppendDrawIndexBufferCounterReset_->SetName(L"ResetAppendDrawIndexBufferCounterReset");
	UINT8* pMappedCounterReset = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	resetAppendDrawIndexBufferCounterReset_->Map(0, &readRange, reinterpret_cast<void**>(&pMappedCounterReset));
	ZeroMemory(pMappedCounterReset, sizeof(UINT));
	resetAppendDrawIndexBufferCounterReset_->Unmap(0, nullptr);


	// DrawIndexの基となるバッファ

	std::vector<uint32_t> commands;
	commands.resize(emitterForGPU_->maxParticleNum);
	desc = CD3DX12_RESOURCE_DESC::Buffer(commandSizePerFrame_);
	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(originalCommandBuffer_.GetAddressOf()));
	originalCommandBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	originalCommandBuffer_->SetName(L"originalCommandBuffer");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = emitterForGPU_->maxParticleNum;
	srvDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	originalCommandSRVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(
		originalCommandBuffer_,
		&srvDesc,
		originalCommandSRVHandle_
	);

	// コピー用
	// インデックスを代入
	UploadBuffer copyIndexBuffer{};
	copyIndexBuffer.Create(L"copyIndex", commandSizePerFrame_);

	for (UINT commandIndex = 0; commandIndex < emitterForGPU_->maxParticleNum; ++commandIndex) {
		commands[commandIndex] = commandIndex;
	}
	copyIndexBuffer.Copy(commands.data(), commandSizePerFrame_);

	commandContext.CopyBuffer(originalCommandBuffer_, copyIndexBuffer);

	// Draw引数用バッファー
	desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(IndirectCommand));
	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(drawArgumentBuffer_.GetAddressOf()));
	drawArgumentBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	drawArgumentBuffer_->SetName(L"DrawArgumentBuffer");
	srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = 1;
	srvDesc.Buffer.StructureByteStride = sizeof(IndirectCommand);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	drawArgumentHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(
		drawArgumentBuffer_,
		&srvDesc,
		drawArgumentHandle_
	);
	UploadBuffer drawCopyBuffer{};
	drawCopyBuffer.Create(L"Copy", sizeof(IndirectCommand));
	IndirectCommand tmp{};
	tmp.srv.particleSRV = particleBuffer_->GetGPUVirtualAddress();
	tmp.srv.drawIndexSRV = appendDrawIndexBuffers_->GetGPUVirtualAddress();
	tmp.drawIndex.IndexCountPerInstance = UINT(6);
	tmp.drawIndex.InstanceCount = 1;
	tmp.drawIndex.BaseVertexLocation = 0;
	tmp.drawIndex.StartIndexLocation = 0;
	tmp.drawIndex.StartInstanceLocation = 0;
	drawCopyBuffer.Copy(&tmp, sizeof(IndirectCommand));
	commandContext.CopyBuffer(drawArgumentBuffer_, drawCopyBuffer);
	// コピー
	commandContext.Close();
	CommandQueue& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue();
	commandQueue.Execute(commandContext);
	commandQueue.Signal();
	commandQueue.WaitForGPU();
	commandContext.Reset();

}

void GPUParticle::InitializeEmitter(EmitterForGPU emitterForGPU) {
	auto device = GraphicsCore::GetInstance()->GetDevice();

	auto desc = CD3DX12_RESOURCE_DESC::Buffer(UINT64(sizeof(EmitterForGPU)),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	GraphicsCore::GetInstance()->GetDevice()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(emitterForGPUBuffer_.GetAddressOf())
	);
	emitterForGPUBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	emitterForGPUBuffer_->SetName(L"emitterForGPUBuffer_");
	emitterForGPU_ = &emitterForGPU;
	UploadBuffer copy{};
	copy.Create(L"Copy", sizeof(EmitterForGPU));
	copy.Copy(emitterForGPU_, sizeof(EmitterForGPU));

	emitterCounter_ = new EmitterCounter();
	emitterCounter_->emitterCounter = 1;
	emitterCounterBuffer_.Create(L"emitterCounterBuffer", sizeof(EmitterCounter));
	emitterCounterBuffer_.Copy(emitterCounter_, sizeof(EmitterCounter));

	auto& commandContext = RenderManager::GetInstance()->GetCommandContext();
	commandContext.CopyBuffer(emitterForGPUBuffer_, copy);

	// コピー
	{
		commandContext.Close();
		CommandQueue& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue();
		commandQueue.Execute(commandContext);
		commandQueue.Signal();
		commandQueue.WaitForGPU();
		commandContext.Reset();
	}
}
