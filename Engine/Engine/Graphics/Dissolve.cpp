#include "Dissolve.h"

#include <d3dx12.h>

#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"

void Dissolve::Initialize(const ColorBuffer& target) {
	{
		CD3DX12_DESCRIPTOR_RANGE textureRange[1]{};
		textureRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE maskTextureRange[1]{};
		maskTextureRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

		CD3DX12_ROOT_PARAMETER rootParameters[RootParameter::kCount]{};
		rootParameters[RootParameter::kTexture].InitAsDescriptorTable(_countof(textureRange), textureRange, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[RootParameter::kMaskTexture].InitAsDescriptorTable(_countof(maskTextureRange), maskTextureRange, D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[RootParameter::kThreshold].InitAsConstantBufferView(0);

		CD3DX12_STATIC_SAMPLER_DESC staticSampler(
			0,
			D3D12_FILTER_MIN_MAG_MIP_LINEAR,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
			D3D12_DEFAULT_MIP_LOD_BIAS, D3D12_DEFAULT_MAX_ANISOTROPY,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.pStaticSamplers = &staticSampler;
		desc.NumStaticSamplers = 1;
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		rootSignature_.Create(L"DissolveRootSigunature", desc);
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

		desc.pRootSignature = rootSignature_;

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = nullptr;
		inputLayoutDesc.NumElements = 0;
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Resources/Shaders/Fullscreen.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Resources/Shaders/Dissolve/Dissolve.PS.hlsl", L"ps_6_0");
		desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
		desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
		desc.BlendState = Helper::BlendAlpha;
		desc.DepthStencilState = Helper::DepthStateDisabled;
		desc.RasterizerState = Helper::RasterizerNoCull;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = target.GetFormat();
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		pipelineState_.Create(L"DissolvePipeLine", desc);
	}
	{
		temporaryBuffer_.Create(L"DissolveTempBuffer", target.GetWidth(), target.GetHeight(), target.GetFormat());
		thresholdBuffer_.Create(L"DissolveThresholdBuffer", sizeof(Desc));
		desc_.threshold = 0.5f;
		desc_.edgeWidth = 0.03f;
		thresholdBuffer_.Copy(&desc_, sizeof(Desc));
		maskTextureHandle_ = TextureManager::GetInstance()->Load("Resources/Images/Dissolve/noise0.png");
	}

}

void Dissolve::Render(CommandContext& commandContext, ColorBuffer& texture) {
	if (isUsed_) {

		thresholdBuffer_.Copy(&desc_, sizeof(Desc));
		commandContext.TransitionResource(QueueType::Type::DIRECT, temporaryBuffer_, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandContext.SetRenderTarget(temporaryBuffer_.GetRTV());
		commandContext.ClearColor(temporaryBuffer_);
		commandContext.SetViewportAndScissorRect(0, 0, temporaryBuffer_.GetWidth(), temporaryBuffer_.GetHeight());

		commandContext.SetGraphicsRootSignature(rootSignature_);
		commandContext.SetPipelineState(QueueType::Type::DIRECT, pipelineState_);

		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandContext.TransitionResource(QueueType::Type::DIRECT, texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandContext.SetGraphicsDescriptorTable(RootParameter::kTexture, texture.GetSRV());
		commandContext.SetGraphicsDescriptorTable(RootParameter::kMaskTexture, TextureManager::GetInstance()->GetTexture(maskTextureHandle_).GetSRV());
		commandContext.SetGraphicsConstantBuffer(RootParameter::kThreshold, thresholdBuffer_.GetGPUVirtualAddress());
		commandContext.Draw(3);

		commandContext.CopyBuffer(QueueType::Type::DIRECT, texture, temporaryBuffer_);
	}
}

void Dissolve::DrawImGui() {
#ifdef ENABLE_IMGUI
	if (ImGui::Begin("RenderManager")) {
		if (ImGui::TreeNode("Dissolve")) {
			ImGui::DragFloat("Threshold", &desc_.threshold, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("EdgeWidth", &desc_.edgeWidth, 0.01f, 0.0f, 1.0f);
			ImGui::TreePop();
		}
		ImGui::End();
	}
#endif // ENABLE_IMGUI
}
