#include "GPUParticle.h"

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

GPUParticle::GPUParticle() {
	InitializeParticleBuffer();
	InitializeUpdateParticle();
	InitializeBuffer();
	InitializeEmitter();
	InitializeAddEmitter();
}

GPUParticle::~GPUParticle() {}

void GPUParticle::Initialize() {}

void GPUParticle::AppendEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty()) {
		// リセット
		addEmitterCopyBuffer_.Copy(nullptr, 0);
		size_t emitterCount = emitterForGPUs_.size();
		size_t copySize = sizeof(GPUParticleShaderStructs::Emitter) * emitterCount;
		addEmitterCopyBuffer_.Copy(emitterForGPUs_.data(), copySize);
		commandContext.CopyBufferRegion(addEmitterBuffer_, 0, addEmitterCopyBuffer_, 0, copySize);

		commandContext.TransitionResource(addEmitterCopyBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(addEmitterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeShaderResource(0, addEmitterCopyBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, addEmitterUAVHandle_);
		commandContext.Dispatch(emitterCount, 1, 1);
	}
}
void GPUParticle::AddEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty()) {
		//// リセット
		//addEmitterCopyBuffer_.Copy(nullptr, 0);
		//size_t copySize = sizeof(GPUParticleShaderStructs::Emitter) * emitterForGPUs_.size();
		//addEmitterCopyBuffer_.Copy(emitterForGPUs_.data(), copySize);
		//commandContext.CopyBufferRegion(addEmitterBuffer_, 0, addEmitterCopyBuffer_, 0, copySize);
		//// 追加するエミッターが何個あるか
		//UINT emitterCount = (static_cast<UINT>(emitterForGPUs_.size()));
		//addEmitterCountBuffer_.Copy(&emitterCount, sizeof(UINT));
		//// カウンターに追加するパーティクルの個数をセット
		//commandContext.CopyBufferRegion(addEmitterBuffer_, addEmitterCounterOffset_, addEmitterCountBuffer_, 0, sizeof(UINT));

		commandContext.TransitionResource(addEmitterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeDescriptorTable(0, addEmitterUAVHandle_);
		commandContext.SetComputeUAV(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
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
	commandContext.CopyBufferRegion(createParticleCounterCopyDestBuffer_, 0, createParticleCounterCopySrcBuffer_, 0, sizeof(UINT));
}

void GPUParticle::Spawn(CommandContext& commandContext) {
	if (*static_cast<uint32_t*>(createParticleCounterCopyDestBuffer_.GetCPUData()) != 0/* &&
		*static_cast<uint32_t*>(originalCommandCounterBuffer_.GetCPUData()) <= GPUParticleShaderStructs::MaxParticleNum*/) {
		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.UAVBarrier(particleBuffer_);
		commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(2, originalCommandUAVHandle_);
		commandContext.SetComputeUAV(3, createParticleBuffer_->GetGPUVirtualAddress());

		commandContext.Dispatch(static_cast<UINT>(ceil(*static_cast<uint32_t*> (createParticleCounterCopyDestBuffer_.GetCPUData()) / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);

		commandContext.CopyBufferRegion(originalCommandCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));
		commandContext.CopyBuffer(createParticleBuffer_, resetCreateParticleBuffer_);
	}
}


void GPUParticle::ParticleUpdate(CommandContext& commandContext) {
	if (*static_cast<uint32_t*>(originalCommandCounterBuffer_.GetCPUData()) != 0) {
		// リセット
		commandContext.CopyBufferRegion(drawIndexCommandBuffers_, particleIndexCounterOffset_, resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.UAVBarrier(particleBuffer_);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandUAVHandle_);
		commandContext.SetComputeDescriptorTable(2, drawIndexCommandUAVHandle_);

		commandContext.Dispatch(static_cast<UINT>(ceil(GPUParticleShaderStructs::MaxParticleNum / float(GPUParticleShaderStructs::ComputeThreadBlockSize))), 1, 1);

		UINT64 destInstanceCountArgumentOffset = sizeof(GPUParticleShaderStructs::IndirectCommand::SRV) + sizeof(UINT);
		UINT64 srcInstanceCountArgumentOffset = particleIndexCounterOffset_;

		commandContext.CopyBufferRegion(drawArgumentBuffer_, destInstanceCountArgumentOffset, drawIndexCommandBuffers_, srcInstanceCountArgumentOffset, sizeof(UINT));
	}
}

void GPUParticle::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	if (*static_cast<uint32_t*>(originalCommandCounterBuffer_.GetCPUData()) != 0) {
		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(drawArgumentBuffer_, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.SetGraphicsConstantBuffer(2, viewProjection.constBuff_.GetGPUVirtualAddress());
		commandContext.SetGraphicsDescriptorTable(3, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());
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
}

void GPUParticle::Create(const GPUParticleShaderStructs::Emitter& emitterForGPU) {
	emitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetEmitter(const GPUParticleShaderStructs::Emitter& emitterForGPU) {
	emitterForGPUs_.emplace_back(emitterForGPU);
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

	originalCommandCounterBuffer_.Create(L"originalCommandCounterBuffer", sizeof(resetValue));
	originalCommandCounterBuffer_.Copy(resetValue);

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
		CD3DX12_ROOT_PARAMETER rootParameters[kCount]{};
		rootParameters[kOriginalBuffer].InitAsUnorderedAccessView(0);
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

	commandContext.SetComputeUAV(kOriginalBuffer, originalCommandBuffer_->GetGPUVirtualAddress());
	commandContext.Dispatch(static_cast<UINT>(ceil(GPUParticleShaderStructs::MaxParticleNum / float(GPUParticleShaderStructs::ComputeThreadBlockSize))), 1, 1);

	// カウンターを初期化
	UploadBuffer copyDrawIndexCounterBuffer{};
	UINT counterNum = GPUParticleShaderStructs::MaxParticleNum;
	copyDrawIndexCounterBuffer.Create(L"copyDrawIndexCounterBuffer", sizeof(counterNum));
	copyDrawIndexCounterBuffer.Copy(counterNum);

	commandContext.CopyBufferRegion(originalCommandBuffer_, particleIndexCounterOffset_, copyDrawIndexCounterBuffer, 0, sizeof(UINT));

	UploadBuffer drawCopyBuffer{};
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
		(sizeof(GPUParticleShaderStructs::Emitter) * GPUParticleShaderStructs::MaxEmitterNum),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);
	// EmitterのIndexをAppend,Consumeするよう
	resetCreateParticleBuffer_.Create(
		L"resetCreateParticleBuffer",
		emitterIndexCounterOffset_ + sizeof(UINT)
	);
	resetCreateParticleBuffer_.Copy(0);
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
	UINT resetValue = 0;
	createParticleCounterCopyDestBuffer_.Create(
		L"CreateParticleCounterCopyDestBuffer",
		sizeof(resetValue)
	);
	createParticleCounterCopyDestBuffer_.Copy(resetValue);
}

void GPUParticle::InitializeAddEmitter() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();


	addEmitterSize_ = sizeof(GPUParticleShaderStructs::Emitter) * GPUParticleShaderStructs::MaxEmitterNum;
	addEmitterCounterOffset_ = AlignForUavCounter(addEmitterSize_);

	addEmitterBuffer_.Create(
		L"AddEmitterBuffer",
		addEmitterCounterOffset_ + sizeof(UINT),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
	);

	addEmitterCopyBuffer_.Create(L"AddEmitterCopyBuffer", addEmitterSize_);

	addEmitterUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// UAVを作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = GPUParticleShaderStructs::MaxEmitterNum;
	uavDesc.Buffer.StructureByteStride = sizeof(GPUParticleShaderStructs::Emitter);
	uavDesc.Buffer.CounterOffsetInBytes = addEmitterCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	device->CreateUnorderedAccessView(
		addEmitterBuffer_,
		addEmitterBuffer_,
		&uavDesc,
		addEmitterUAVHandle_);

	addEmitterCountBuffer_.Create(L"AddEmitterCounterBuffer", sizeof(UINT));
	addEmitterCountBuffer_.Copy(0);
}
