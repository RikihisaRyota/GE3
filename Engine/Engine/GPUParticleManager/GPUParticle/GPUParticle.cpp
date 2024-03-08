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

void GPUParticle::EmitterUpdate(CommandContext& commandContext) {
	// createParticleのリセット
	commandContext.CopyBufferRegion(createParticleBuffer_, emitterIndexCounterOffset_, resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));
	commandContext.CopyBufferRegion(createParticleCounterCopySrcBuffer_, 0, resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));

	commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createParticleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.TransitionResource(createParticleCounterCopySrcBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.SetComputeUAV(0, emitterForGPUBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeDescriptorTable(1, createParticleUAVHandle_);
	commandContext.SetComputeUAV(2, createParticleCounterCopySrcBuffer_->GetGPUVirtualAddress());

	commandContext.Dispatch(1, 1, 1);
	commandContext.CopyBufferRegion(createParticleCounterCopyDestBuffer_, 0, createParticleCounterCopySrcBuffer_, 0, sizeof(UINT));
	createParticleCounter_ = static_cast<uint32_t*>(createParticleCounterDate_);
}

void GPUParticle::AddEmitter(CommandContext& commandContext) {
	// エミッター追加
	if (!emitterForGPUs_.empty()) {
		// リセット
		addEmitterCopyBuffer_.Copy(nullptr, 0);
		addEmitterCopyBuffer_.Copy(emitterForGPUs_.data(), addEmitterCounterOffset_);
		commandContext.CopyBufferRegion(addEmitterBuffer_, 0, addEmitterCopyBuffer_, 0, addEmitterCounterOffset_);
		// 追加するエミッターが何個あるか
		UINT emitterCount = (static_cast<UINT>(emitterForGPUs_.size()));
		addEmitterCountBuffer_.Copy(&emitterCount, sizeof(UINT));
		// カウンターに追加するパーティクルの個数をセット
		commandContext.CopyBufferRegion(addEmitterBuffer_, addEmitterCounterOffset_, addEmitterCountBuffer_, 0, sizeof(UINT));

		commandContext.TransitionResource(addEmitterBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeDescriptorTable(0, addEmitterUAVHandle_);
		commandContext.SetComputeUAV(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
		commandContext.Dispatch(1, 1, 1);
		emitterForGPUs_.clear();
	}
}

void GPUParticle::Spawn(CommandContext& commandContext) {
	if (*createParticleCounter_ != 0 &&
		*originalCommandCounter_ < MaxParticleNum) {
		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(emitterForGPUBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeShaderResource(1, emitterForGPUBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(2, originalCommandUAVHandle_);
		commandContext.SetComputeUAV(3, createParticleBuffer_->GetGPUVirtualAddress());

		commandContext.Dispatch(static_cast<UINT>(ceil(*createParticleCounter_ / ComputeThreadBlockSize)), 1, 1);
		commandContext.CopyBufferRegion(originalCommandCounterBuffer_, 0, originalCommandBuffer_, particleIndexCounterOffset_, sizeof(UINT));
		originalCommandCounter_ = static_cast<uint32_t*>(originalCommandCounterDate_);
	}
}


void GPUParticle::ParticleUpdate(CommandContext& commandContext) {
	if (*originalCommandCounter_ != 0) {
		// リセット
		commandContext.CopyBufferRegion(drawIndexCommandBuffers_, particleIndexCounterOffset_, resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandUAVHandle_);
		commandContext.SetComputeDescriptorTable(2, drawIndexCommandUAVHandle_);

		commandContext.Dispatch(static_cast<UINT>(ceil(MaxParticleNum / float(ComputeThreadBlockSize))), 1, 1);

		UINT64 destInstanceCountArgumentOffset = sizeof(IndirectCommand::SRV) + sizeof(UINT);
		UINT64 srcInstanceCountArgumentOffset = particleIndexCounterOffset_;

		commandContext.CopyBufferRegion(drawArgumentBuffer_, destInstanceCountArgumentOffset, drawIndexCommandBuffers_, srcInstanceCountArgumentOffset, sizeof(UINT));
	}
}

void GPUParticle::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	if (*originalCommandCounter_ != 0) {
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

void GPUParticle::Create(const EmitterForGPU& emitterForGPU) {
	emitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::SetEmitter(const Emitter& emitterForGPU) {
	emitterForGPUs_.emplace_back(emitterForGPU);
}

void GPUParticle::InitializeParticleBuffer() {
	// ParticleBuffer
	auto device = GraphicsCore::GetInstance()->GetDevice();
	{
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(
			UINT64(sizeof(Particle) * MaxParticleNum),
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

	particleIndexSize_ = MaxParticleNum * sizeof(uint32_t);
	particleIndexCounterOffset_ = AlignForUavCounter(particleIndexSize_);

	// 何番目のパーティクルが生きているか積み込みよう
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(particleIndexCounterOffset_ + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12_HEAP_PROPERTIES heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(drawIndexCommandBuffers_.GetAddressOf())
	);
	drawIndexCommandBuffers_.SetState(D3D12_RESOURCE_STATE_COMMON);
	drawIndexCommandBuffers_->SetName(L"DrawIndexBuffers");
	drawIndexCommandUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// UAVView生成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = MaxParticleNum;
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


	desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT));
	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(originalCommandCounterBuffer_.GetAddressOf())
	);
	originalCommandCounterBuffer_->SetName(L"originalCommandCounterBuffer");
	originalCommandCounterBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	originalCommandCounterBuffer_->Map(0, nullptr, &originalCommandCounterDate_);
	originalCommandCounter_ = new uint32_t();
	*originalCommandCounter_ = 0;

	// パーティクルのindexをAppend,Consumeするよう
	desc = CD3DX12_RESOURCE_DESC::Buffer(particleIndexCounterOffset_ + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
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
	originalCommandUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// リソースビューを作成した後、UAV を作成
	uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = MaxParticleNum;
	uavDesc.Buffer.StructureByteStride = sizeof(uint32_t);
	uavDesc.Buffer.CounterOffsetInBytes = particleIndexCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	device->CreateUnorderedAccessView(
		originalCommandBuffer_,
		originalCommandBuffer_,
		&uavDesc,
		originalCommandUAVHandle_);

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
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
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
	commandContext.Dispatch(static_cast<UINT>(ceil(MaxParticleNum / float(ComputeThreadBlockSize))), 1, 1);

	// カウンターを初期化
	UploadBuffer copyDrawIndexCounterBuffer{};
	UINT counterNum = MaxParticleNum;
	copyDrawIndexCounterBuffer.Create(L"copyDrawIndexCounterBuffer", sizeof(counterNum));
	copyDrawIndexCounterBuffer.Copy(counterNum);

	commandContext.CopyBufferRegion(originalCommandBuffer_, particleIndexCounterOffset_, copyDrawIndexCounterBuffer, 0, sizeof(UINT));

	UploadBuffer drawCopyBuffer{};
	drawCopyBuffer.Create(L"Copy", sizeof(IndirectCommand));
	IndirectCommand tmp{};
	tmp.srv.particleSRV = particleBuffer_->GetGPUVirtualAddress();
	tmp.srv.drawIndexSRV = drawIndexCommandBuffers_->GetGPUVirtualAddress();
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

void GPUParticle::InitializeEmitter() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();

	emitterIndexSize_ = MaxParticleNum * sizeof(CreateParticle);
	emitterIndexCounterOffset_ = AlignForUavCounter(emitterIndexSize_);

	// エミッターバッファー
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(
		UINT64(sizeof(EmitterForGPU) * MaxEmitterNum),
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(emitterForGPUBuffer_.GetAddressOf()));
	emitterForGPUBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	emitterForGPUBuffer_->SetName(L"EmitterForGPUBuffer");

	// EmitterのIndexをAppend,Consumeするよう
	desc = CD3DX12_RESOURCE_DESC::Buffer(emitterIndexCounterOffset_ + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(createParticleBuffer_.GetAddressOf()));
	createParticleBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	createParticleBuffer_->SetName(L"CreateParticleBuffer");
	createParticleUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// UAVを作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = MaxEmitterNum;
	uavDesc.Buffer.StructureByteStride = sizeof(CreateParticle);
	uavDesc.Buffer.CounterOffsetInBytes = emitterIndexCounterOffset_;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
	device->CreateUnorderedAccessView(
		createParticleBuffer_,
		createParticleBuffer_,
		&uavDesc,
		createParticleUAVHandle_);

	desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(createParticleCounterCopySrcBuffer_.GetAddressOf())
	);
	createParticleCounterCopySrcBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	createParticleCounterCopySrcBuffer_->SetName(L"CreateParticleCounterCopySrcBuffer");


	desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(UINT));
	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(createParticleCounterCopyDestBuffer_.GetAddressOf())
	);
	createParticleCounterCopyDestBuffer_->SetName(L"CreateParticleCounterCopyDestBuffer");
	createParticleCounterCopyDestBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	createParticleCounterCopyDestBuffer_->Map(0, nullptr, &createParticleCounterDate_);
	createParticleCounter_ = new uint32_t();
	*createParticleCounter_ = 0;
}

void GPUParticle::InitializeAddEmitter() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();


	addEmitterSize_ = sizeof(EmitterForGPU) * MaxEmitterNum;
	addEmitterCounterOffset_ = AlignForUavCounter(addEmitterSize_);

	auto desc = CD3DX12_RESOURCE_DESC::Buffer(addEmitterCounterOffset_ + sizeof(UINT), D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	device->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(addEmitterBuffer_.GetAddressOf()));
	addEmitterBuffer_.SetState(D3D12_RESOURCE_STATE_COMMON);
	addEmitterBuffer_->SetName(L"AddEmitterBuffer");

	addEmitterCopyBuffer_.Create(L"AddEmitterCopyBuffer", addEmitterCounterOffset_);

	addEmitterUAVHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// UAVを作成
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = MaxEmitterNum;
	uavDesc.Buffer.StructureByteStride = sizeof(EmitterForGPU);
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
