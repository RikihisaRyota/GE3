#include "GPUParticleManager.h"

#include <d3dx12.h>

#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/CommandContext.h"

namespace ParticleManager {
	enum SpawnRootSignature {
		kParticleInfo,
		kEmitterSRV,
		kOutputCommand,
		kCreateParticle,

		kSpawnRootSignatureCount,
	};

	enum EmitterUpdateRootSigunature {
		kEmitterUAV,
		kCreateParticleNum,
		kCounterBuffer,

		kEmitterUpdateRootSigunatureCount,
	};

	enum AddEmitterRootSigunature {
		kAddEmitter,
		kOriginalEmitter,

		kAddEmitterRootSigunatureCount,
	};

	enum ParticleUpdateRootSigunature {
		kParticleBuffer,
		kParticleIndexCommand,
		kOutputDrawIndex,

		kUpdateRootSigunatureCount,
	};

	enum CommandSigunature {
		kParticleSRV,
		kDrawIndexSRV,
		kDrawIndexBuffer,

		kCommandSigunatureCount,
	};

	enum GraphicsSigunature {
		kParticle,
		kDrawIndex,
		kViewProjection,
		kTexture,
		kSampler,

		kGraphicsSigunatureCount,
	};
}

void GPUParticleManager::Initialize() {
	CreateParticleBuffer();
	CreateEmitter();
	CreateSpawn();
	CreateUpdate();
	CreateGraphics();
	CreateAddEmitter();
	CreateIndexBuffer();
	gpuParticle_->SetCommandSignature(commandSignature_.Get());
}

void GPUParticleManager::Update(CommandContext& commandContext) {

	commandContext.SetComputeRootSignature(*addEmitterComputeRootSignature_);
	commandContext.SetPipelineState(*addEmitterComputePipelineState_);
	gpuParticle_->AddEmitter(commandContext);

	commandContext.SetComputeRootSignature(*emitterUpdateComputeRootSignature_);
	commandContext.SetPipelineState(*emitterUpdateComputePipelineState_);
	gpuParticle_->EmitterUpdate(commandContext);

	commandContext.SetComputeRootSignature(*spawnComputeRootSignature_);
	commandContext.SetPipelineState(*spawnComputePipelineState_);
	gpuParticle_->Spawn(commandContext);

	commandContext.SetComputeRootSignature(*updateComputeRootSignature_);
	commandContext.SetPipelineState(*updateComputePipelineState_);
	gpuParticle_->ParticleUpdate(commandContext);
}

void GPUParticleManager::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	commandContext.SetGraphicsRootSignature(*graphicsRootSignature_);
	commandContext.SetPipelineState(*graphicsPipelineState_);

	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext.SetVertexBuffer(0, vbView_);
	commandContext.SetIndexBuffer(ibView_);
	gpuParticle_->Draw(viewProjection, commandContext);
}

void GPUParticleManager::CreateParticle(const Emitter& emitterForGPU) {
	gpuParticle_->Create(emitterForGPU);
}

void GPUParticleManager::CreateParticleBuffer() {
	gpuParticle_ = std::make_unique<GPUParticle>();
	gpuParticle_->Initialize();
}

void GPUParticleManager::CreateGraphics() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();

	// グラフィックスルートシグネイチャ
	{
		graphicsRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_DESCRIPTOR_RANGE textureRange[1]{};
		textureRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, graphics->kNumSRVs, 0, 1);

		CD3DX12_DESCRIPTOR_RANGE samplerRanges[1]{};
		samplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[ParticleManager::GraphicsSigunature::kGraphicsSigunatureCount]{};
		rootParameters[ParticleManager::GraphicsSigunature::kParticle].InitAsShaderResourceView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[ParticleManager::GraphicsSigunature::kDrawIndex].InitAsShaderResourceView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[ParticleManager::GraphicsSigunature::kViewProjection].InitAsConstantBufferView(0);
		rootParameters[ParticleManager::GraphicsSigunature::kTexture].InitAsDescriptorTable(_countof(textureRange), textureRange, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[ParticleManager::GraphicsSigunature::kSampler].InitAsDescriptorTable(_countof(samplerRanges), samplerRanges, D3D12_SHADER_VISIBILITY_PIXEL);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		graphicsRootSignature_->Create(L"GPUParticle RootSignature", desc);
	}
	// コマンドシグネイチャ
	{
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[ParticleManager::CommandSigunature::kCommandSigunatureCount] = {};
		argumentDescs[ParticleManager::CommandSigunature::kParticleSRV].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argumentDescs[ParticleManager::CommandSigunature::kParticleSRV].ShaderResourceView.RootParameterIndex = 0;
		argumentDescs[ParticleManager::CommandSigunature::kDrawIndexSRV].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argumentDescs[ParticleManager::CommandSigunature::kDrawIndexSRV].ShaderResourceView.RootParameterIndex = 1;
		argumentDescs[ParticleManager::CommandSigunature::kDrawIndexBuffer].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc{};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(IndirectCommand);
		auto result = device->CreateCommandSignature(&commandSignatureDesc, *graphicsRootSignature_, IID_PPV_ARGS(&commandSignature_));
		commandSignature_->SetName(L"commandSignature");
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
		graphicsPipelineState_->Create(L"GPUParticle PSO", desc);
	}
}

void GPUParticleManager::CreateEmitter() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	// アップデートシグネイチャー
	{
		emitterUpdateComputeRootSignature_ = std::make_unique<RootSignature>();

		//	createParticle用
		CD3DX12_DESCRIPTOR_RANGE addParticleRange[1]{};
		addParticleRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, ParticleManager::EmitterUpdateRootSigunature::kCreateParticleNum, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[ParticleManager::EmitterUpdateRootSigunature::kEmitterUpdateRootSigunatureCount]{};
		rootParameters[ParticleManager::EmitterUpdateRootSigunature::kEmitterUAV].InitAsUnorderedAccessView(0);
		rootParameters[ParticleManager::EmitterUpdateRootSigunature::kCreateParticleNum].InitAsDescriptorTable(_countof(addParticleRange), addParticleRange);
		rootParameters[ParticleManager::EmitterUpdateRootSigunature::kCounterBuffer].InitAsUnorderedAccessView(2);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		emitterUpdateComputeRootSignature_->Create(L"GPUParticle AddEmitterCPSO", desc);
	}
	// アップデートパイプライン
	{
		emitterUpdateComputePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *emitterUpdateComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/EmitterUpdate.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		emitterUpdateComputePipelineState_->Create(L"GPUParticle AddEmitterCPSO", desc);
	}
}

void GPUParticleManager::CreateAddEmitter() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	// アップデートシグネイチャー
	{
		addEmitterComputeRootSignature_ = std::make_unique<RootSignature>();

		//	createParticle用
		CD3DX12_DESCRIPTOR_RANGE createParticleRange[1]{};
		createParticleRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, ParticleManager::AddEmitterRootSigunature::kAddEmitter, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[ParticleManager::AddEmitterRootSigunature::kAddEmitterRootSigunatureCount]{};
		rootParameters[ParticleManager::AddEmitterRootSigunature::kAddEmitter].InitAsDescriptorTable(_countof(createParticleRange), createParticleRange);
		rootParameters[ParticleManager::AddEmitterRootSigunature::kOriginalEmitter].InitAsUnorderedAccessView(1);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		addEmitterComputeRootSignature_->Create(L"GPUParticle AddEmitterUpdateCPSO", desc);
	}
	// アップデートパイプライン
	{
		addEmitterComputePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *addEmitterComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/AddEmitter.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		addEmitterComputePipelineState_->Create(L"GPUParticle AddEmitterUpdateCPSO", desc);
	}
}

void GPUParticleManager::CreateUpdate() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	// アップデートシグネイチャー
	{
		updateComputeRootSignature_ = std::make_unique<RootSignature>();

		//	ParticleIndexCommand用（カウンター付きUAVの場合このように宣言）
		CD3DX12_DESCRIPTOR_RANGE particleIndexRange[1]{};
		particleIndexRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, ParticleManager::ParticleUpdateRootSigunature::kParticleIndexCommand, 0);
		CD3DX12_DESCRIPTOR_RANGE outputDrawRange[1]{};
		outputDrawRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, ParticleManager::ParticleUpdateRootSigunature::kOutputDrawIndex, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[ParticleManager::ParticleUpdateRootSigunature::kUpdateRootSigunatureCount]{};
		rootParameters[ParticleManager::ParticleUpdateRootSigunature::kParticleBuffer].InitAsUnorderedAccessView(0);
		rootParameters[ParticleManager::ParticleUpdateRootSigunature::kParticleIndexCommand].InitAsDescriptorTable(_countof(particleIndexRange), particleIndexRange);
		rootParameters[ParticleManager::ParticleUpdateRootSigunature::kOutputDrawIndex].InitAsDescriptorTable(_countof(outputDrawRange), outputDrawRange);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		updateComputeRootSignature_->Create(L"GPUParticle UpdateRootSignature", desc);
	}
	// アップデートパイプライン
	{
		updateComputePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *updateComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/Update.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		updateComputePipelineState_->Create(L"GPUParticle UpdateCPSO", desc);
	}
}

void GPUParticleManager::CreateSpawn() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	// スポーンシグネイチャー
	{
		spawnComputeRootSignature_ = std::make_unique<RootSignature>();
		// AppendStructuredBuffer用（カウンター付きUAVの場合このように宣言）
		CD3DX12_DESCRIPTOR_RANGE consumeRanges[1]{};
		consumeRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);
		CD3DX12_ROOT_PARAMETER rootParameters[ParticleManager::SpawnRootSignature::kSpawnRootSignatureCount]{};
		rootParameters[ParticleManager::SpawnRootSignature::kParticleInfo].InitAsUnorderedAccessView(0);
		rootParameters[ParticleManager::SpawnRootSignature::kEmitterSRV].InitAsShaderResourceView(0);
		rootParameters[ParticleManager::SpawnRootSignature::kOutputCommand].InitAsDescriptorTable(_countof(consumeRanges), consumeRanges);
		rootParameters[ParticleManager::SpawnRootSignature::kCreateParticle].InitAsUnorderedAccessView(2);
		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		spawnComputeRootSignature_->Create(L"GPUParticle SpawnRootSignature", desc);
	}
	// スポーンパイプライン
	{
		spawnComputePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *spawnComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/Spawn.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		spawnComputePipelineState_->Create(L"GPUParticle SpawnCPSO", desc);
	}
}

void GPUParticleManager::CreateIndexBuffer() {
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

		vertexBuffer_.Create(L"GPUParticleVertexBuffer", sizeIB);

		vertexBuffer_.Copy(vertices.data(), sizeIB);

		vbView_.BufferLocation = vertexBuffer_.GetGPUVirtualAddress();
		vbView_.SizeInBytes = UINT(sizeIB);
		vbView_.StrideInBytes = sizeof(Vertex);
	}
	// インデックスバッファ
	{
		std::vector<uint16_t>indices = {
		0, 1, 3,
		2, 0, 3,
		};

		size_t sizeIB = sizeof(uint16_t) * indices.size();

		indexBuffer_.Create(L"GPUParticleIndexBuffer", sizeIB);

		indexBuffer_.Copy(indices.data(), sizeIB);

		// インデックスバッファビューの作成
		ibView_.BufferLocation = indexBuffer_.GetGPUVirtualAddress();
		ibView_.Format = DXGI_FORMAT_R16_UINT;
		ibView_.SizeInBytes = UINT(sizeIB);
	}
}
