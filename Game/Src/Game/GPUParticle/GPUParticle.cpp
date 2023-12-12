#include "GPUParticle.h"

#include <d3dx12.h>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Model/ModelManager.h"

GPUParticle::GPUParticle() {
	graphicsPipelineState_ = std::make_unique<PipelineState>();
	graphicsRootSignature_ = std::make_unique<RootSignature>();

	spawnComputePipelineState_ = std::make_unique<PipelineState>();
	spawnComputeRootSignature_ = std::make_unique<RootSignature>();

	updateComputePipelineState_ = std::make_unique<PipelineState>();
	updateComputeRootSignature_ = std::make_unique<RootSignature>();

	InitializeSpawnParticle();

	InitializeUpdateParticle();

	InitializeGraphics();

	modelHandle_ = ModelManager::GetInstance()->Load("Game/Resources/Models/teapot");
	worldTransform_.Initialize();
}

void GPUParticle::Initialize() {

}

void GPUParticle::Update() {

	auto commandContext = RenderManager::GetInstance()->GetCommandContext();
	commandContext.TransitionResource(rwStructuredBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.SetPipelineState(*updateComputePipelineState_);
	commandContext.SetComputeRootSignature(*updateComputeRootSignature_);
	commandContext.SetComputeUAV(0, rwStructuredBuffer_->GetGPUVirtualAddress());
	commandContext.SetComputeConstantBuffer(1, updateConstantBuffer_->GetGPUVirtualAddress());
	commandContext.Dispatch(kNumThread, 1, 1);

}

void GPUParticle::Render(const ViewProjection& viewProjection) {
	auto commandContext = RenderManager::GetInstance()->GetCommandContext();
	
	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection, modelHandle_, commandContext);

	commandContext.SetPipelineState(*graphicsPipelineState_);
	commandContext.SetGraphicsRootSignature(*graphicsRootSignature_);
	commandContext.SetVertexBuffer(0, vbView_);
	commandContext.SetIndexBuffer(ibView_);
	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandContext.TransitionResource(rwStructuredBuffer_, D3D12_RESOURCE_STATE_GENERIC_READ);
	commandContext.SetGraphicsDescriptorTable(0, rwStructuredBufferHandle_);
	commandContext.SetGraphicsConstantBuffer(1, viewProjection.constBuff_->GetGPUVirtualAddress());
	commandContext.DrawIndexedInstanced(static_cast<UINT>(indices_.size()), kNumThread);
}

void GPUParticle::InitializeSpawnParticle() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();

	// RWStructuredBuffer
	{
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(
			UINT64(sizeof(Particle) * kNumThread),
			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
		device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(rwStructuredBuffer_.GetAddressOf())
		);
	}
	// 初期化用コンピュートルートシグネイチャ
	{
		CD3DX12_ROOT_PARAMETER rootParameters[1]{};
		rootParameters[0].InitAsUnorderedAccessView(0);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		spawnComputeRootSignature_->Create(L"GPUParticle SpawnRootSignature", desc);

	}
	// 初期化用コンピュートパイプライン
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *spawnComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Game/Resources/Shaders/Spawn.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		spawnComputePipelineState_->Create(L"GPUParticle SpawnCPSO", desc);
	}
	// 初期化
	{
		auto commandContext = RenderManager::GetInstance()->GetCommandContext();
		commandContext.SetPipelineState(*spawnComputePipelineState_);
		commandContext.SetComputeRootSignature(*spawnComputeRootSignature_);

		commandContext.TransitionResource(rwStructuredBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		commandContext.SetComputeUAV(0, rwStructuredBuffer_->GetGPUVirtualAddress());

		commandContext.Dispatch(kNumThread, 1, 1);
		commandContext.Close();

		CommandQueue& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue();
		commandQueue.Execute(commandContext);
		commandQueue.Signal();
		commandQueue.WaitForGPU();
		commandContext.Reset();
	}
}

void GPUParticle::InitializeUpdateParticle() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	// アップデート用コンピュートシグネイチャ
	{
		CD3DX12_ROOT_PARAMETER rootParameters[2]{};
		rootParameters[0].InitAsUnorderedAccessView(0);
		rootParameters[1].InitAsConstantBufferView(0);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		updateComputeRootSignature_->Create(L"GPUParticle UpdateRootSignature", desc);
	}
	// アップデート用コンピュートパイプライン
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *updateComputeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Game/Resources/Shaders/Update.CS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		updateComputePipelineState_->Create(L"GPUParticle UpdateCPSO", desc);
	}
	// UAVの生成
	{
		uavHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = kNumThread;
		desc.Buffer.StructureByteStride = sizeof(Particle);
		device->CreateUnorderedAccessView(
			rwStructuredBuffer_,
			nullptr,
			&desc,
			uavHandle_
		);
	}
	// 定数バッファ
	{
		updateConstantBuffer_.Create(L"GPUParticle UpdateConstantBuffer", sizeof(particleInfo_));
		particleInfo_ = new ParticleInfo();
		particleInfo_->speed = 0.5f;
		updateConstantBuffer_.Copy(particleInfo_, sizeof(particleInfo_));
	}
	// マップ
	{
		/*rwStructuredBuffer_->Map(0, 0, reinterpret_cast<void**>(&updateParticle_));
		memset(updateParticle_, 0, sizeof(uint32_t) * kNumThread);*/
	}
}

void GPUParticle::InitializeGraphics() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	// グラフィックスルートシグネイチャ
	{
		CD3DX12_DESCRIPTOR_RANGE range{};
		range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[2]{};
		rootParameters[0].InitAsDescriptorTable(1, &range);
		rootParameters[1].InitAsConstantBufferView(0);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		graphicsRootSignature_->Create(L"GPUParticle RootSignature", desc);
	}
	// グラフィックスパイプライン
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

		desc.pRootSignature = *graphicsRootSignature_;

		D3D12_INPUT_ELEMENT_DESC inputElements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElements;
		inputLayoutDesc.NumElements = _countof(inputElements);
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Game/Resources/Shaders/GPUParticle.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Game/Resources/Shaders/GPUParticle.PS.hlsl", L"ps_6_0");
		desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
		desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
		desc.BlendState = Helper::BlendDisable;
		desc.DepthStencilState = Helper::DepthStateReadWrite;
		desc.RasterizerState = Helper::RasterizerDefault;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = RenderManager::GetInstance()->GetRenderTargetFormat();
		desc.DSVFormat = RenderManager::GetInstance()->GetDepthFormat();
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		graphicsPipelineState_->Create(L"GPUParticle PSO", desc);
	}
	// シェーダーリソース用
	{
		rwStructuredBufferHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = kNumThread;
		desc.Buffer.StructureByteStride = sizeof(Particle);
		device->CreateShaderResourceView(rwStructuredBuffer_, &desc, rwStructuredBufferHandle_);
	}
	// 頂点バッファ
	{
		struct Vertex {
			Vector4 position;
		};
		std::vector<Vertex>vertex = {
		// 前
		{-0.5f, -0.5f, -0.5f, 1.0f}, // 左下
		{-0.5f, +0.5f, -0.5f, 1.0f}, // 左上
		{+0.5f, -0.5f, -0.5f, 1.0f}, // 右下
		{+0.5f, +0.5f, -0.5f, 1.0f}, // 右上
		// 後(前面とZ座標の符号が逆)
		{+0.5f, -0.5f, +0.5f, 1.0f}, // 左下
		{+0.5f, +0.5f, +0.5f, 1.0f}, // 左上
		{-0.5f, -0.5f, +0.5f, 1.0f}, // 右下
		{-0.5f, +0.5f, +0.5f, 1.0f}, // 右上
		// 左
		{-0.5f, -0.5f, +0.5f, 1.0f}, // 左下
		{-0.5f, +0.5f, +0.5f, 1.0f}, // 左上
		{-0.5f, -0.5f, -0.5f, 1.0f}, // 右下
		{-0.5f, +0.5f, -0.5f, 1.0f}, // 右上
		// 右（左面とX座標の符号が逆）
		{+0.5f, -0.5f, -0.5f, 1.0f}, // 左下
		{+0.5f, +0.5f, -0.5f, 1.0f}, // 左上
		{+0.5f, -0.5f, +0.5f, 1.0f}, // 右下
		{+0.5f, +0.5f, +0.5f, 1.0f}, // 右上
		// 下
		{+0.5f, -0.5f, -0.5f, 1.0f}, // 左下
		{+0.5f, -0.5f, +0.5f, 1.0f}, // 左上
		{-0.5f, -0.5f, -0.5f, 1.0f}, // 右下
		{-0.5f, -0.5f, +0.5f, 1.0f}, // 右上
		// 上（下面とY座標の符号が逆）
		{-0.5f, +0.5f, -0.5f, 1.0f}, // 左下
		{-0.5f, +0.5f, +0.5f, 1.0f}, // 左上
		{+0.5f, +0.5f, -0.5f, 1.0f}, // 右下
		{+0.5f, +0.5f, +0.5f, 1.0f}, // 右上
		};
		
		size_t sizeIB = sizeof(vertex.at(0)) * vertex.size();

		vertexBuffer_.Create(L"vertexBuffer", sizeIB);

		vertexBuffer_.Copy(vertex.data(), sizeIB);

		vbView_.BufferLocation = vertexBuffer_.GetGPUVirtualAddress();
		vbView_.SizeInBytes = UINT(vertexBuffer_.GetBufferSize());
		vbView_.StrideInBytes = sizeof(vertex.at(0));
	}
	// インデックスバッファ
	{
		indices_ = {
			0,  1,  3,
			3,  2,  0,
			4,  5,  7,
			7,  6,  4,
			8,  9,  11,
			11, 10, 8,
			12, 13, 15,
			15, 14, 12,
			16, 17, 19,
			19, 18, 16,
			20, 21, 23,
			23, 22, 20
		};

		size_t sizeIB = sizeof(uint16_t) * indices_.size();

		indexBuffer_.Create(L"indexBuffer", sizeIB);

		indexBuffer_.Copy(indices_.data(), sizeIB);

		// インデックスバッファビューの作成
		ibView_.BufferLocation = indexBuffer_.GetGPUVirtualAddress();
		ibView_.Format = DXGI_FORMAT_R16_UINT;
		ibView_.SizeInBytes = UINT(indexBuffer_.GetBufferSize());
	}
}
