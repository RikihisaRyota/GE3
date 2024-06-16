#include "Skybox.h"

#include <d3dx12.h>

#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"

namespace {
		enum Param {
			kWorldTransform,
			kViewProjection,
			kTexture,
			kCount,
		};
}

Skybox::Skybox() {
	{
		rootSignature_ = std::make_unique<RootSignature>();
		CD3DX12_DESCRIPTOR_RANGE range[1]{};
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[Param::kCount]{};
		rootParameters[Param::kWorldTransform].InitAsConstantBufferView(0);
		rootParameters[Param::kViewProjection].InitAsConstantBufferView(1);
		rootParameters[Param::kTexture].InitAsDescriptorTable(_countof(range), range);

		CD3DX12_STATIC_SAMPLER_DESC samplerDesc
			= CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);
		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		desc.NumStaticSamplers = 1;
		desc.pStaticSamplers = &samplerDesc;
		rootSignature_->Create(L"SkyboxRootSignature", desc);
	}
	{
		pipelineState_ = std::make_unique<PipelineState>();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

		desc.pRootSignature = *rootSignature_;

		D3D12_INPUT_ELEMENT_DESC inputElements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElements;
		inputLayoutDesc.NumElements = _countof(inputElements);
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Resources/Shaders/Skybox/Skybox.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Resources/Shaders/Skybox/Skybox.PS.hlsl", L"ps_6_0");
		desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
		desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
		desc.BlendState = Helper::BlendAlpha;
		desc.DepthStencilState.DepthEnable = true;
		desc.DepthStencilState.DepthWriteMask= D3D12_DEPTH_WRITE_MASK_ZERO;
		desc.DepthStencilState.DepthFunc= D3D12_COMPARISON_FUNC_LESS_EQUAL;
		desc.RasterizerState = Helper::RasterizerDefault;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = RenderManager::GetInstance()->GetRenderTargetFormat();
		desc.DSVFormat = RenderManager::GetInstance()->GetDepthFormat();
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		pipelineState_->Create(L"Skybox PSO", desc);
	}
	{
		vertices_ = {
			// 右面
			{1.0f, 1.0f, 1.0f, 1.0f},    // 0
			{1.0f, -1.0f, -1.0f, 1.0f},  // 3
			{1.0f, -1.0f, 1.0f, 1.0f},   // 2
			{1.0f, 1.0f, 1.0f, 1.0f},    // 0
			{1.0f, 1.0f, -1.0f, 1.0f},   // 1
			{1.0f, -1.0f, -1.0f, 1.0f},  // 3

			// 左面
			{-1.0f, 1.0f, -1.0f, 1.0f},  // 4
			{-1.0f, -1.0f, 1.0f, 1.0f},  // 7
			{-1.0f, -1.0f, -1.0f, 1.0f}, // 6
			{-1.0f, 1.0f, -1.0f, 1.0f},  // 4
			{-1.0f, 1.0f, 1.0f, 1.0f},   // 5
			{-1.0f, -1.0f, 1.0f, 1.0f},  // 7

			// 前面
			{-1.0f, 1.0f, 1.0f, 1.0f},   // 8
			{1.0f, -1.0f, 1.0f, 1.0f},   // 11
			{-1.0f, -1.0f, 1.0f, 1.0f},  // 10
			{-1.0f, 1.0f, 1.0f, 1.0f},   // 8
			{1.0f, 1.0f, 1.0f, 1.0f},    // 9
			{1.0f, -1.0f, 1.0f, 1.0f},   // 11

			// 後面
			{1.0f, 1.0f, -1.0f, 1.0f},   // 12
			{-1.0f, -1.0f, -1.0f, 1.0f}, // 15
			{1.0f, -1.0f, -1.0f, 1.0f},  // 14
			{1.0f, 1.0f, -1.0f, 1.0f},   // 12
			{-1.0f, 1.0f, -1.0f, 1.0f},  // 13
			{-1.0f, -1.0f, -1.0f, 1.0f}, // 15

			// 上面
			{-1.0f, 1.0f, 1.0f, 1.0f},   // 16
			{1.0f, 1.0f, -1.0f, 1.0f},   // 19
			{1.0f, 1.0f, 1.0f, 1.0f},    // 18
			{-1.0f, 1.0f, 1.0f, 1.0f},   // 16
			{-1.0f, 1.0f, -1.0f, 1.0f},  // 17
			{1.0f, 1.0f, -1.0f, 1.0f},   // 19

			// 下面
			{-1.0f, -1.0f, -1.0f, 1.0f}, // 20
			{1.0f, -1.0f, 1.0f, 1.0f},   // 23
			{1.0f, -1.0f, -1.0f, 1.0f},  // 22
			{-1.0f, -1.0f, -1.0f, 1.0f}, // 20
			{-1.0f, -1.0f, 1.0f, 1.0f},  // 21
			{1.0f, -1.0f, 1.0f, 1.0f},   // 23
		};


		vertBuff_.Create(L"SkyboxVertexBuffer", vertices_.size()*sizeof(Vector4));
		// 頂点バッファビューの作成
		vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
		vbView_.SizeInBytes = UINT(vertices_.size() * sizeof(Vector4));
		vbView_.StrideInBytes = sizeof(vertices_[0]);
		vertBuff_.Copy(vertices_.data(), UINT(vertices_.size() * sizeof(Vector4)));
	}
	{
		textureHandle_ = TextureManager::GetInstance()->Load("Resources/Images/Skybox/rostock_laage_airport_4k.dds");
		worldTransform_.Initialize();
		
		worldTransform_.UpdateMatrix();
	}
}

void Skybox::Initialize() {
	worldTransform_.Reset();
	worldTransform_.scale = { 30.0f,30.0f,30.0f };
	worldTransform_.UpdateMatrix();
}

void Skybox::Update() {
	worldTransform_.UpdateMatrix();
}


void Skybox::Draw(CommandContext& commandContext, const ViewProjection& viewProjection) {
	commandContext.SetGraphicsRootSignature(*rootSignature_);
	commandContext.SetPipelineState(*pipelineState_);

	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandContext.SetVertexBuffer(0, vbView_);

	commandContext.SetGraphicsConstantBuffer(Param::kWorldTransform, worldTransform_.constBuff.get()->GetGPUVirtualAddress());
	commandContext.SetGraphicsConstantBuffer(Param::kViewProjection, viewProjection.constBuff_.GetGPUVirtualAddress());
	commandContext.SetGraphicsDescriptorTable(Param::kTexture, TextureManager::GetInstance()->GetTexture(textureHandle_).GetSRV());

	commandContext.Draw(static_cast<UINT>(vertices_.size()));
}

void Skybox::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Skybox")) {
		ImGui::DragFloat3("scale", &worldTransform_.scale.x, 1.0f, 0.0f);
		ImGui::EndMenu();
	}
	ImGui::End();
}
