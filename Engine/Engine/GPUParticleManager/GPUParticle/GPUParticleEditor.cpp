#include "GPUParticleEditor.h"

#include <d3dx12.h>

#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Math/ViewProjection.h"

namespace GPUParticleEditorParameter {
	enum Spawn {
		kEmitter,
		kParticleIndex,
		kConsumeParticleIndex,

		kSpawCount,
	};
	enum CommandSigunature {
		kParticleSRV,
		kDrawIndexSRV,
		kDrawIndexBuffer,

		kCommandSigunatureCount,
	};

	enum Graphics {
		kParticle,
		kDrawIndex,
		kViewProjection,
		kTexture,
		kSampler,

		kGraphicsSigunatureCount,
	};
}

void GPUParticleEditor::Initialize() {
	CreateSpawn();
	CreateGraphics();
	CreateIndexBuffer();

	CreateParticleBuffer();
	CreateEmitterBuffer();
	CreateUpdateParticle();
	CreateBuffer();
}

void GPUParticleEditor::Spawn(CommandContext& commandContext) {
	if (emitter_.frequency.time <= 0) {
		commandContext.SetComputeRootSignature(*spawnComputeRootSignature_);
		commandContext.SetPipelineState(*spawnComputePipelineState_);
		commandContext.SetComputeConstantBuffer(GPUParticleEditorParameter::Spawn::kEmitter, emitterBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeUAV(GPUParticleEditorParameter::Spawn::kParticleIndex, particleBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(GPUParticleEditorParameter::Spawn::kConsumeParticleIndex, originalCommandUAVHandle_);
		commandContext.Dispatch(static_cast<UINT>(ceil(emitter_.createParticleNum / GPUParticleShaderStructs::ComputeThreadBlockSize)), 1, 1);
	}
}

void GPUParticleEditor::ParticleUpdate(CommandContext& commandContext) {
	if (*static_cast<uint32_t*>(originalCommandCounterBuffer_.GetCPUData()) != 0) {
		commandContext.SetComputeRootSignature(*updateComputeRootSignature_);
		commandContext.SetPipelineState(*updateComputePipelineState_);
		// リセット
		commandContext.CopyBufferRegion(drawIndexCommandBuffers_, particleIndexCounterOffset_, resetAppendDrawIndexBufferCounterReset_, 0, sizeof(UINT));

		commandContext.TransitionResource(particleBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.TransitionResource(drawIndexCommandBuffers_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		commandContext.SetComputeUAV(0, particleBuffer_->GetGPUVirtualAddress());
		commandContext.SetComputeDescriptorTable(1, originalCommandUAVHandle_);
		commandContext.SetComputeDescriptorTable(2, drawIndexCommandUAVHandle_);

		commandContext.Dispatch(static_cast<UINT>(ceil(GPUParticleShaderStructs::MaxParticleNum / float(GPUParticleShaderStructs::ComputeThreadBlockSize))), 1, 1);

		UINT64 destInstanceCountArgumentOffset = sizeof(IndirectCommand::SRV) + sizeof(UINT);
		UINT64 srcInstanceCountArgumentOffset = particleIndexCounterOffset_;

		commandContext.CopyBufferRegion(drawArgumentBuffer_, destInstanceCountArgumentOffset, drawIndexCommandBuffers_, srcInstanceCountArgumentOffset, sizeof(UINT));
	}
}

void GPUParticleEditor::Update(CommandContext& commandContext) {
	Spawn(commandContext);
	ParticleUpdate(commandContext);
}

void GPUParticleEditor::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	if (static_cast<uint32_t*>(originalCommandCounterBuffer_.GetCPUData()) != 0) {
		commandContext.SetGraphicsRootSignature(*graphicsRootSignature_);
		commandContext.SetPipelineState(*graphicsPipelineState_);
		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.SetGraphicsConstantBuffer(2, viewProjection.constBuff_.GetGPUVirtualAddress());
		commandContext.SetGraphicsDescriptorTable(3, GraphicsCore::GetInstance()->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV).GetStartDescriptorHandle());
		commandContext.SetGraphicsDescriptorTable(4, SamplerManager::LinearWrap);
		commandContext.ExecuteIndirect(
			commandSignature_.Get(),
			1,
			drawArgumentBuffer_,
			0,
			nullptr,
			0
		);
	}
}
void GPUParticleEditor::CreateParticle(const Emitter& emitterForGPU) {}

void GPUParticleEditor::CreateGraphics() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();

	// グラフィックスルートシグネイチャ
	{
		graphicsRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_DESCRIPTOR_RANGE textureRange[1]{};
		textureRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, graphics->kNumSRVs, 0, 1);

		CD3DX12_DESCRIPTOR_RANGE samplerRanges[1]{};
		samplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[GPUParticleEditorParameter::Graphics::kGraphicsSigunatureCount]{};
		rootParameters[GPUParticleEditorParameter::Graphics::kParticle].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[GPUParticleEditorParameter::Graphics::kDrawIndex].InitAsShaderResourceView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[GPUParticleEditorParameter::Graphics::kViewProjection].InitAsConstantBufferView(0);
		rootParameters[GPUParticleEditorParameter::Graphics::kTexture].InitAsDescriptorTable(_countof(textureRange), textureRange, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[GPUParticleEditorParameter::Graphics::kSampler].InitAsDescriptorTable(_countof(samplerRanges), samplerRanges, D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		graphicsRootSignature_->Create(L"EditorGPUParticle RootSignature", desc);
	}
	// コマンドシグネイチャ
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[GPUParticleEditorParameter::CommandSigunature::kCommandSigunatureCount] = {};
		argumentDescs[GPUParticleEditorParameter::CommandSigunature::kParticleSRV].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argumentDescs[GPUParticleEditorParameter::CommandSigunature::kParticleSRV].ShaderResourceView.RootParameterIndex = 0;
		argumentDescs[GPUParticleEditorParameter::CommandSigunature::kDrawIndexSRV].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argumentDescs[GPUParticleEditorParameter::CommandSigunature::kDrawIndexSRV].ShaderResourceView.RootParameterIndex = 1;
		argumentDescs[GPUParticleEditorParameter::CommandSigunature::kDrawIndexBuffer].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc{};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(IndirectCommand);
		auto result = device->CreateCommandSignature(&commandSignatureDesc, *graphicsRootSignature_, IID_PPV_ARGS(&commandSignature_));
		commandSignature_->SetName(L"EditorCommandSignature");
		assert(SUCCEEDED(result));
	}
	// グラフィックスパイプライン
	{
		graphicsPipelineState_ = std::make_unique<PipelineState>();
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

		desc.pRootSignature = *graphicsRootSignature_;

		D3D12_INPUT_ELEMENT_DESC inputElements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElements;
		inputLayoutDesc.NumElements = _countof(inputElements);
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/GPUParticle.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/GPUParticle.PS.hlsl", L"ps_6_0");
		desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
		desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
		desc.BlendState = Helper::BlendAdditive;
		desc.DepthStencilState = Helper::DepthStateRead;
		desc.RasterizerState = Helper::RasterizerNoCull;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = RenderManager::GetInstance()->GetRenderTargetFormat();
		desc.DSVFormat = RenderManager::GetInstance()->GetDepthFormat();
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		graphicsPipelineState_->Create(L"EditorGPUParticle PSO", desc);
	}
}

void GPUParticleEditor::CreateSpawn() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	// アップデートシグネイチャー
	{
		emitterUpdateComputeRootSignature_ = std::make_unique<RootSignature>();

		//	ParticleIndexCommand用（カウンター付きUAVの場合このように宣言）
		CD3DX12_DESCRIPTOR_RANGE consumeRanges[1]{};
		consumeRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[GPUParticleEditorParameter::Spawn::kSpawCount]{};
		rootParameters[GPUParticleEditorParameter::Spawn::kEmitter].InitAsConstantBufferView(0);
		rootParameters[GPUParticleEditorParameter::Spawn::kConsumeParticleIndex].InitAsDescriptorTable(_countof(consumeRanges), consumeRanges);
		rootParameters[GPUParticleEditorParameter::Spawn::kParticleIndex].InitAsUnorderedAccessView(1);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		emitterUpdateComputeRootSignature_->Create(L"EditorSpawnRootsignature", desc);
	}
	// アップデートパイプライン
	{
		emitterUpdateComputePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *emitterUpdateComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/Editor/EditorEmitterSpaw.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		emitterUpdateComputePipelineState_->Create(L"EditorEmitterCS", desc);
	}
}

void GPUParticleEditor::CreateIndexBuffer() {
	// 頂点バッファ
	{
		struct Vertex {
			Vector3 position;
			Vector2 texcoord;
		};
		std::vector<Vertex> vertices = {
			// 前
			{ { -0.5f, -0.5f, +0.0f },{0.0f,1.0f} }, // 左下
			{ { -0.5f, +0.5f, +0.0f },{0.0f,0.0f} }, // 左上
			{ { +0.5f, -0.5f, +0.0f },{1.0f,1.0f} }, // 右下
			{ { +0.5f, +0.5f, +0.0f },{1.0f,0.0f} }, // 右上
		};

		size_t sizeIB = sizeof(Vertex) * vertices.size();

		vertexBuffer_.Create(L"EditorGPUParticleVertexBuffer", sizeIB);

		vertexBuffer_.Copy(vertices.data(), sizeIB);

		vbView_.BufferLocation = vertexBuffer_.GetGPUVirtualAddress();
		vbView_.SizeInBytes = UINT(sizeIB);
		vbView_.StrideInBytes = sizeof(Vertex);
	}
	// インデックスバッファ
	{
		std::vector<uint32_t>indices = {
		0, 1, 3,
		2, 0, 3,
		};

		size_t sizeIB = sizeof(uint32_t) * indices.size();

		indexBuffer_.Create(L"EditorGPUParticleIndexBuffer", sizeIB);

		indexBuffer_.Copy(indices.data(), sizeIB);

		// インデックスバッファビューの作成
		ibView_.BufferLocation = indexBuffer_.GetGPUVirtualAddress();
		ibView_.Format = DXGI_FORMAT_R16_UINT;
		ibView_.SizeInBytes = UINT(sizeIB);
	}
}

void GPUParticleEditor::CreateEmitterBuffer() {

	emitterBuffer_.Create(L"EmitterBuffer", sizeof(Emitter));
	emitterBuffer_.Copy(emitter_);
}

void GPUParticleEditor::CreateParticleBuffer() {
	particleBuffer_.Create(L"EditorParticleBuffer", sizeof(Particle) * GPUParticleShaderStructs::MaxParticleNum, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
}

void GPUParticleEditor::CreateUpdateParticle() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto& commandContext = RenderManager::GetInstance()->GetCommandContext();

	particleIndexSize_ = GPUParticleShaderStructs::MaxParticleNum * sizeof(uint32_t);
	particleIndexCounterOffset_ = AlignForUavCounter(particleIndexSize_);

	// 何番目のパーティクルが生きているか積み込みよう
	drawIndexCommandBuffers_.Create(
		L"EditorDrawIndexBuffers",
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
	resetAppendDrawIndexBufferCounterReset_.Create(L"EditorResetAppendDrawIndexBufferCounterReset", sizeof(resetValue));
	resetAppendDrawIndexBufferCounterReset_.Copy(resetValue);

	originalCommandCounterBuffer_.Create(L"EditorOriginalCommandCounterBuffer", sizeof(UINT));
	originalCommandCounterBuffer_.Copy(0);

	// パーティクルのindexをAppend,Consumeするよう
	originalCommandBuffer_.Create(
		L"EditorOriginalCommandBuffer",
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
		L"EditorDrawArgumentBuffer",
		sizeof(IndirectCommand)
	);

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

void GPUParticleEditor::CreateBuffer() {
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

		initializeBufferRootSignature.Create(L"EditorInitializeBufferRootSignature", desc);
	}
	// スポーンパイプライン
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = initializeBufferRootSignature;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/InitializeGPUParticle.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		initializeBufferPipelineState.Create(L"EditorInitializeBufferCPSO", desc);
	}
	commandContext.SetComputeRootSignature(initializeBufferRootSignature);
	commandContext.SetPipelineState(initializeBufferPipelineState);

	commandContext.TransitionResource(originalCommandBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	commandContext.SetComputeUAV(kOriginalBuffer, originalCommandBuffer_->GetGPUVirtualAddress());
	commandContext.Dispatch(static_cast<UINT>(ceil(GPUParticleShaderStructs::MaxParticleNum / float(GPUParticleShaderStructs::ComputeThreadBlockSize))), 1, 1);

	// カウンターを初期化
	UploadBuffer copyDrawIndexCounterBuffer{};
	UINT counterNum = GPUParticleShaderStructs::MaxParticleNum;
	copyDrawIndexCounterBuffer.Create(L"EditorcopyDrawIndexCounterBuffer", sizeof(counterNum));
	copyDrawIndexCounterBuffer.Copy(counterNum);

	commandContext.CopyBufferRegion(originalCommandBuffer_, particleIndexCounterOffset_, copyDrawIndexCounterBuffer, 0, sizeof(UINT));

	UploadBuffer drawCopyBuffer{};
	drawCopyBuffer.Create(L"EditorCopy", sizeof(IndirectCommand));
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
