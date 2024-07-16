#include "GPUParticleManager.h"

#include <chrono>
#include <ctime>

#include <d3dx12.h>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Math/WorldTransform.h"


namespace ParticleManager {
	enum SpawnRootSignature {
		kParticleInfo,
		kEmitterSRV,
		//kEmitterCount,
		kRandomBuffer,
		kOutputCommand,
		kCreateParticle,
		kSpawnCounterBuffer,

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
		kEmitterCounter,

		kAddEmitterRootSigunatureCount,
	};

	enum ParticleUpdateRootSigunature {
		kViewPrijection,
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

	enum BulletForGPUSigunature {
		kBullets,
		kBulletNum,
		kAllParticle,
		kRandomBuff,
		kBulletForGPUSigunatureCount,
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
	CreateBullet();
	CreateMeshParticle();
	CreateField();
	randomBuffer_.Create(L"rondomBuffer", sizeof(UINT));
	gpuParticle_->SetDrawCommandSignature(commandSignature_.get());
	gpuParticle_->SetSpawnCommandSignature(spawnCommandSignature_.get());
}

void GPUParticleManager::Update(const ViewProjection& viewProjection, CommandContext& commandContext) {
	UINT seed = static_cast<UINT>(std::chrono::system_clock::now().time_since_epoch().count());
	randomBuffer_.Copy(&seed, sizeof(UINT));

	commandContext.SetComputeRootSignature(*checkFieldRootSignature_);
	commandContext.SetPipelineState(*checkFieldPipelineState_);
	gpuParticle_->CheckField(commandContext);

	commandContext.SetComputeRootSignature(*addFieldRootSignature_);
	commandContext.SetPipelineState(*addFieldPipelineState_);
	gpuParticle_->AddField(commandContext);

	commandContext.SetComputeRootSignature(*checkEmitterComputeRootSignature_);
	commandContext.SetPipelineState(*checkEmitterComputePipelineState_);
	gpuParticle_->CheckEmitter(commandContext);

	commandContext.SetComputeRootSignature(*addEmitterComputeRootSignature_);
	commandContext.SetPipelineState(*addEmitterComputePipelineState_);
	gpuParticle_->AddEmitter(commandContext);

	commandContext.SetComputeRootSignature(*emitterUpdateComputeRootSignature_);
	commandContext.SetPipelineState(*emitterUpdateComputePipelineState_);
	gpuParticle_->EmitterUpdate(commandContext);

	commandContext.SetComputeRootSignature(*spawnComputeRootSignature_);
	commandContext.SetPipelineState(*spawnComputePipelineState_);
	gpuParticle_->Spawn(commandContext, randomBuffer_);

	commandContext.SetComputeRootSignature(*updateComputeRootSignature_);
	commandContext.SetPipelineState(*updateComputePipelineState_);
	gpuParticle_->ParticleUpdate(viewProjection, commandContext);

	commandContext.SetComputeRootSignature(*bulletRootSignature_);
	commandContext.SetPipelineState(*bulletPipelineState_);
	gpuParticle_->BulletUpdate(commandContext, randomBuffer_);

	commandContext.SetComputeRootSignature(*updateFieldRootSignature_);
	commandContext.SetPipelineState(*updateFieldPipelineState_);
	gpuParticle_->UpdateField(commandContext);

	commandContext.SetComputeRootSignature(*collisionFieldRootSignature_);
	commandContext.SetPipelineState(*collisionFieldPipelineState_);
	gpuParticle_->CollisionField(commandContext);
}

void GPUParticleManager::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	//commandContext.TransitionResource(RenderManager::GetInstance()->GetMainDepthBuffer(), D3D12_RESOURCE_STATE_DEPTH_READ);
	//commandContext.FlushResourceBarriers();

	//commandContext.SetRenderTarget(RenderManager::GetInstance()->GetMainColorBuffer().GetRTV(), RenderManager::GetInstance()->GetMainDepthBuffer().GetReadOnlyDSV());

	commandContext.SetGraphicsRootSignature(*graphicsRootSignature_);
	commandContext.SetPipelineState(*graphicsPipelineState_);

	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
	commandContext.SetDynamicVertexBuffer(0, vertices.size(), sizeof(vertices.at(0)), vertices.data());
	std::vector<uint16_t>indices = {
		0, 1, 3,
		2, 0, 3,
	};
	commandContext.SetDynamicIndexBuffer(indices.size(), DXGI_FORMAT_R16_UINT, indices.data());
	gpuParticle_->Draw(viewProjection, commandContext);
	//commandContext.TransitionResource(RenderManager::GetInstance()->GetMainDepthBuffer(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
	//commandContext.FlushResourceBarriers();
}

void GPUParticleManager::DrawImGui() {
	gpuParticle_->DrawImGui();
}

void GPUParticleManager::CreateMeshParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const WorldTransform& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, CommandContext& commandContext) {
	commandContext.SetComputeRootSignature(*meshParticleRootSignature_);
	commandContext.SetPipelineState(*meshParticlePipelineState_);

	gpuParticle_->CreateMeshParticle(modelHandle, animation, worldTransform, mesh, randomBuffer_, commandContext);
}

void GPUParticleManager::CreateMeshParticle(const ModelHandle& modelHandle, const WorldTransform& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, CommandContext& commandContext) {
	commandContext.SetComputeRootSignature(*meshParticleRootSignature_);
	commandContext.SetPipelineState(*meshParticlePipelineState_);

	gpuParticle_->CreateMeshParticle(modelHandle, worldTransform, mesh, randomBuffer_, commandContext);
}

void GPUParticleManager::CreateVertexParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const WorldTransform& worldTransform, const GPUParticleShaderStructs::VertexEmitterDesc& mesh, CommandContext& commandContext) {
	commandContext.SetComputeRootSignature(*vertexParticleRootSignature_);
	commandContext.SetPipelineState(*vertexParticlePipelineState_);

	gpuParticle_->CreateVertexParticle(modelHandle, animation, worldTransform, mesh, randomBuffer_, commandContext);
}

void GPUParticleManager::CreateVertexParticle(const ModelHandle& modelHandle, const WorldTransform& worldTransform, const GPUParticleShaderStructs::VertexEmitterDesc& mesh, CommandContext& commandContext) {
	commandContext.SetComputeRootSignature(*edgeParticleRootSignature_);
	commandContext.SetPipelineState(*edgeParticlePipelineState_);

	gpuParticle_->CreateVertexParticle(modelHandle, worldTransform, mesh, randomBuffer_, commandContext);
}

void GPUParticleManager::CreateEdgeParticle(const ModelHandle& modelHandle, Animation::Animation& animation, const WorldTransform& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, CommandContext& commandContext) {
	commandContext.SetComputeRootSignature(*edgeParticleRootSignature_);
	commandContext.SetPipelineState(*edgeParticlePipelineState_);

	gpuParticle_->CreateEdgeParticle(modelHandle, animation, worldTransform, mesh, randomBuffer_, commandContext);
}

void GPUParticleManager::CreateEdgeParticle(const ModelHandle& modelHandle, const WorldTransform& worldTransform, const GPUParticleShaderStructs::MeshEmitterDesc& mesh, CommandContext& commandContext) {
	commandContext.SetComputeRootSignature(*edgeParticleRootSignature_);
	commandContext.SetPipelineState(*edgeParticlePipelineState_);

	gpuParticle_->CreateEdgeParticle(modelHandle, worldTransform, mesh, randomBuffer_, commandContext);
}

void GPUParticleManager::SetEmitter(const GPUParticleShaderStructs::EmitterForCPU& emitter) {
	gpuParticle_->SetEmitter(emitter);
}

void GPUParticleManager::SetField(const GPUParticleShaderStructs::FieldForCPU& fieldForCPU) {
	gpuParticle_->SetField(fieldForCPU);
}

void GPUParticleManager::SetBullet(const GPUParticleShaderStructs::BulletForGPU& bullets) {
	gpuParticle_->SetBullet(bullets);
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
		commandSignature_ = std::make_unique<CommandSignature>();
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[ParticleManager::CommandSigunature::kCommandSigunatureCount] = {};
		argumentDescs[ParticleManager::CommandSigunature::kParticleSRV].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argumentDescs[ParticleManager::CommandSigunature::kParticleSRV].ShaderResourceView.RootParameterIndex = 0;
		argumentDescs[ParticleManager::CommandSigunature::kDrawIndexSRV].Type = D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW;
		argumentDescs[ParticleManager::CommandSigunature::kDrawIndexSRV].ShaderResourceView.RootParameterIndex = 1;
		argumentDescs[ParticleManager::CommandSigunature::kDrawIndexBuffer].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc{};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(GPUParticleShaderStructs::IndirectCommand);
		commandSignature_->Create(L"commandSignature", commandSignatureDesc, graphicsRootSignature_.get());
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

		emitterUpdateComputeRootSignature_->Create(L"EmitterUpdateComputeRootSignature", desc);
	}
	// アップデートパイプライン
	{
		emitterUpdateComputePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *emitterUpdateComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/EmitterUpdate.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		emitterUpdateComputePipelineState_->Create(L"EmitterUpdateComputePipelineState", desc);
	}
	// アップデートシグネイチャー
	{
		checkEmitterComputeRootSignature_ = std::make_unique<RootSignature>();

		//	createParticle用
		CD3DX12_DESCRIPTOR_RANGE addParticleRange[1]{};
		addParticleRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, ParticleManager::EmitterUpdateRootSigunature::kCreateParticleNum, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[2]{};
		rootParameters[0].InitAsUnorderedAccessView(0);
		rootParameters[1].InitAsUnorderedAccessView(1);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		checkEmitterComputeRootSignature_->Create(L"CheckEmitterComputeRootSignature", desc);
	}
	// アップデートパイプライン
	{
		checkEmitterComputePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *checkEmitterComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/CheckEmitter.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		checkEmitterComputePipelineState_->Create(L"CheckEmitterComputePipelineState", desc);
	}
}

void GPUParticleManager::CreateAddEmitter() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	// アップデートシグネイチャー
	{
		addEmitterComputeRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_ROOT_PARAMETER rootParameters[ParticleManager::AddEmitterRootSigunature::kAddEmitterRootSigunatureCount]{};
		rootParameters[ParticleManager::AddEmitterRootSigunature::kAddEmitter].InitAsShaderResourceView(0);
		rootParameters[ParticleManager::AddEmitterRootSigunature::kOriginalEmitter].InitAsUnorderedAccessView(0);
		rootParameters[ParticleManager::AddEmitterRootSigunature::kEmitterCounter].InitAsUnorderedAccessView(1);

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
		particleIndexRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE outputDrawRange[1]{};
		outputDrawRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[ParticleManager::ParticleUpdateRootSigunature::kUpdateRootSigunatureCount]{};
		rootParameters[ParticleManager::ParticleUpdateRootSigunature::kViewPrijection].InitAsConstantBufferView(0);
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
		//rootParameters[ParticleManager::SpawnRootSignature::kEmitterCount].InitAsConstantBufferView(0);
		rootParameters[ParticleManager::SpawnRootSignature::kRandomBuffer].InitAsConstantBufferView(0);
		rootParameters[ParticleManager::SpawnRootSignature::kOutputCommand].InitAsDescriptorTable(_countof(consumeRanges), consumeRanges);
		rootParameters[ParticleManager::SpawnRootSignature::kCreateParticle].InitAsUnorderedAccessView(2);
		rootParameters[ParticleManager::SpawnRootSignature::kSpawnCounterBuffer].InitAsUnorderedAccessView(3);
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
	// コマンドシグネイチャ
	{
		spawnCommandSignature_ = std::make_unique<CommandSignature>();
		D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
		argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc{};
		commandSignatureDesc.pArgumentDescs = argumentDescs;
		commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
		commandSignatureDesc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
		spawnCommandSignature_->Create(L"spawnCommandSignature", commandSignatureDesc, nullptr);
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

void GPUParticleManager::CreateBullet() {
	{
		bulletRootSignature_ = std::make_unique<RootSignature>();
		CD3DX12_DESCRIPTOR_RANGE bulletRange[1]{};
		bulletRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		CD3DX12_ROOT_PARAMETER rootParameters[ParticleManager::BulletForGPUSigunature::kBulletForGPUSigunatureCount]{};
		rootParameters[ParticleManager::BulletForGPUSigunature::kBullets].InitAsDescriptorTable(_countof(bulletRange), bulletRange);
		rootParameters[ParticleManager::BulletForGPUSigunature::kBulletNum].InitAsConstantBufferView(0);
		rootParameters[ParticleManager::BulletForGPUSigunature::kAllParticle].InitAsUnorderedAccessView(0);
		rootParameters[ParticleManager::BulletForGPUSigunature::kRandomBuff].InitAsConstantBufferView(1);
		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		bulletRootSignature_->Create(L"BulletRootSignature", desc);
	}
	{
		bulletPipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *bulletRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/CollisionBullet.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		bulletPipelineState_->Create(L"CollisionBullet CPSO", desc);
	}
}
void GPUParticleManager::CreateMeshParticle() {
	{
		enum {
			kParticle,
			kParticleIndex,
			kParticleIndexCounter,
			kVertices,
			kIndices,
			kRandom,
			kWorldTransform,
			kIndexCount,
			kMeshEmitter,
			kCount,
		};

		meshParticleRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_DESCRIPTOR_RANGE particleIndexCommandsRange[1]{};
		particleIndexCommandsRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[kCount]{};

		rootParameters[kParticle].InitAsUnorderedAccessView(0);
		rootParameters[kParticleIndex].InitAsDescriptorTable(_countof(particleIndexCommandsRange), particleIndexCommandsRange);
		rootParameters[kParticleIndexCounter].InitAsUnorderedAccessView(2);
		rootParameters[kVertices].InitAsShaderResourceView(0);
		rootParameters[kIndices].InitAsShaderResourceView(1);
		rootParameters[kRandom].InitAsConstantBufferView(0);
		rootParameters[kWorldTransform].InitAsConstantBufferView(1);
		rootParameters[kIndexCount].InitAsConstantBufferView(2);
		rootParameters[kMeshEmitter].InitAsConstantBufferView(3);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		meshParticleRootSignature_->Create(L"MeshParticleRootSignature", desc);
	}
	{
		meshParticlePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *meshParticleRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/MeshParticle.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		meshParticlePipelineState_->Create(L"MeshParticle CPSO", desc);
	}

	{
		enum {
			kParticle,
			kParticleIndex,
			kParticleIndexCounter,
			kVertices,
			kRandom,
			kWorldTransform,
			kVertexCount,
			kMeshEmitter,
			kCount,
		};

		vertexParticleRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_DESCRIPTOR_RANGE particleIndexCommandsRange[1]{};
		particleIndexCommandsRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[kCount]{};

		rootParameters[kParticle].InitAsUnorderedAccessView(0);
		rootParameters[kParticleIndex].InitAsDescriptorTable(_countof(particleIndexCommandsRange), particleIndexCommandsRange);
		rootParameters[kParticleIndexCounter].InitAsUnorderedAccessView(2);
		rootParameters[kVertices].InitAsShaderResourceView(0);
		rootParameters[kRandom].InitAsConstantBufferView(0);
		rootParameters[kWorldTransform].InitAsConstantBufferView(1);
		rootParameters[kVertexCount].InitAsConstantBufferView(2);
		rootParameters[kMeshEmitter].InitAsConstantBufferView(3);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		vertexParticleRootSignature_->Create(L"VertexParticleRootSignature", desc);
	}
	{
		vertexParticlePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *vertexParticleRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/VertexParticle.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		vertexParticlePipelineState_->Create(L"VertexParticle CPSO", desc);
	}

	{
		enum {
			kParticle,
			kParticleIndex,
			kParticleIndexCounter,
			kVertices,
			kIndices,
			kRandom,
			kWorldTransform,
			kIndexCount,
			kMeshEmitter,
			kCount,
		};

		edgeParticleRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_DESCRIPTOR_RANGE particleIndexCommandsRange[1]{};
		particleIndexCommandsRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[kCount]{};

		rootParameters[kParticle].InitAsUnorderedAccessView(0);
		rootParameters[kParticleIndex].InitAsDescriptorTable(_countof(particleIndexCommandsRange), particleIndexCommandsRange);
		rootParameters[kParticleIndexCounter].InitAsUnorderedAccessView(2);
		rootParameters[kVertices].InitAsShaderResourceView(0);
		rootParameters[kIndices].InitAsShaderResourceView(1);
		rootParameters[kRandom].InitAsConstantBufferView(0);
		rootParameters[kWorldTransform].InitAsConstantBufferView(1);
		rootParameters[kIndexCount].InitAsConstantBufferView(2);
		rootParameters[kMeshEmitter].InitAsConstantBufferView(3);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		edgeParticleRootSignature_->Create(L"EdgeParticleRootSignature", desc);
	}
	{
		edgeParticlePipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *edgeParticleRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/EdgeParticle.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		edgeParticlePipelineState_->Create(L"EdgeParticle CPSO", desc);
	}

}

void GPUParticleManager::CreateField() {
	{
		checkFieldRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_ROOT_PARAMETER rootParameters[2]{};
		rootParameters[0].InitAsUnorderedAccessView(0);
		rootParameters[1].InitAsUnorderedAccessView(1);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		checkFieldRootSignature_->Create(L"checkFieldRootSignature", desc);
	}
	// アップデートパイプライン
	{
		checkFieldPipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *checkFieldRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/CheckField.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		checkFieldPipelineState_->Create(L"checkFieldPipelineState", desc);
	}
	{
		addFieldRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_DESCRIPTOR_RANGE fieldIndexStockBuffer[1]{};
		fieldIndexStockBuffer[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[4]{};
		rootParameters[0].InitAsShaderResourceView(0);
		rootParameters[1].InitAsUnorderedAccessView(0);
		rootParameters[2].InitAsUnorderedAccessView(1);
		rootParameters[3].InitAsDescriptorTable(_countof(fieldIndexStockBuffer), fieldIndexStockBuffer);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		addFieldRootSignature_->Create(L"addFieldRootSignature", desc);
	}
	// アップデートパイプライン
	{
		addFieldPipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *addFieldRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/AddField.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		addFieldPipelineState_->Create(L"addFieldPipelineState", desc);
	}
	{
		updateFieldRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_DESCRIPTOR_RANGE fieldIndexStockBuffer[1]{};
		fieldIndexStockBuffer[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE fieldIndexBuffer[1]{};
		fieldIndexBuffer[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2, 0);
		
		CD3DX12_ROOT_PARAMETER rootParameters[3]{};
		rootParameters[0].InitAsUnorderedAccessView(0);
		rootParameters[1].InitAsDescriptorTable(_countof(fieldIndexStockBuffer), fieldIndexStockBuffer);
		rootParameters[2].InitAsDescriptorTable(_countof(fieldIndexBuffer), fieldIndexBuffer);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		updateFieldRootSignature_->Create(L"fieldUpdateRootSignature", desc);
	}
	// アップデートパイプライン
	{
		updateFieldPipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *updateFieldRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/UpdateField.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		updateFieldPipelineState_->Create(L"addFieldPipelineState", desc);
	}
	{
		collisionFieldRootSignature_ = std::make_unique<RootSignature>();

		CD3DX12_ROOT_PARAMETER rootParameters[3]{};
		rootParameters[0].InitAsShaderResourceView(0);
		rootParameters[1].InitAsShaderResourceView(1);
		rootParameters[2].InitAsUnorderedAccessView(0);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		collisionFieldRootSignature_->Create(L"CollisionFieldRootSignature", desc);
	}
	// アップデートパイプライン
	{
		collisionFieldPipelineState_ = std::make_unique<PipelineState>();
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *collisionFieldRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Resources/Shaders/GPUParticle/CollisionField.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		collisionFieldPipelineState_->Create(L"addFieldPipelineState", desc);
	}

}
