#include "Bloom.h"

#include <d3dx12.h>

#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/ImGui/ImGuiManager.h"

void Bloom::Initialize(const ColorBuffer& target) {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	{
		CD3DX12_DESCRIPTOR_RANGE ranges[kMaxLevel + 1]{};
		for (uint32_t i = 0; i < kMaxLevel + 1; ++i) {
			ranges[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i);
		}

		CD3DX12_ROOT_PARAMETER rootParameters[kMaxLevel + 1 + 1]{};
		for (uint32_t i = 0; i < kMaxLevel + 1; ++i) {
			rootParameters[i].InitAsDescriptorTable(1, &ranges[i]);
		}
		rootParameters[RootParameter::kDesc].InitAsConstantBufferView(0);
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

		rootSignature_.Create(L"BloomRootSigunature", desc);
	}
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

		desc.pRootSignature = rootSignature_;

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = nullptr;
		inputLayoutDesc.NumElements = 0;
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Resources/Shaders/Fullscreen.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Resources/Shaders/Bloom/Bloom.PS.hlsl", L"ps_6_0");
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
		pipelineState_.Create(L"BloomPipeLine", desc);
	}
	{
		temporaryRenderBuffer_.Create(L"BloomTempBuffer", target.GetWidth(), target.GetHeight(), target.GetFormat());
		temporaryBuffer_.Create(L"BloomTempBuffer", target.GetWidth(), target.GetHeight(), target.GetFormat());

	}

	gaussianFilter_[0].Initialize(target);
	for (uint32_t i = 1; i < kMaxLevel; i++) {
		gaussianFilter_[i].Initialize(gaussianFilter_[i - 1].GetTemporaryBuffer());
	}
}

void Bloom::Render(CommandContext& commandContext, ColorBuffer& texture) {

	commandContext.CopyBuffer(temporaryBuffer_, texture);
	gaussianFilter_[0].Render(commandContext, texture);
	for (uint32_t i = 1; i < kMaxLevel; i++) {
		gaussianFilter_[i].Render(commandContext, gaussianFilter_[i - 1].GetTemporaryBuffer());
	}
	commandContext.TransitionResource(temporaryRenderBuffer_, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.SetRenderTarget(temporaryRenderBuffer_.GetRTV());
	commandContext.ClearColor(temporaryRenderBuffer_);
	commandContext.SetViewportAndScissorRect(0, 0, temporaryRenderBuffer_.GetWidth(), temporaryRenderBuffer_.GetHeight());

	commandContext.SetGraphicsRootSignature(rootSignature_);
	commandContext.SetPipelineState(pipelineState_);

	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandContext.TransitionResource(texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	for (uint32_t i = 0; i < kMaxLevel; ++i) {
		commandContext.SetGraphicsDescriptorTable(i, gaussianFilter_[i].GetTemporaryBuffer().GetSRV());
	}
	commandContext.SetGraphicsDescriptorTable(RootParameter::kOriginal, temporaryBuffer_.GetSRV());
	commandContext.SetGraphicsDynamicConstantBufferView(RootParameter::kDesc, sizeof(Desc), &desc_);
	commandContext.Draw(3);

	commandContext.CopyBuffer(texture, temporaryRenderBuffer_);
}

void Bloom::Debug() {
#ifdef _DEBUG
	ImGui::Begin("Effect");
	if (ImGui::TreeNode("Bloom")) {
		ImGui::DragFloat("Intensity", &desc_.intensity, 0.01f, 0.0f, 1.0f);
		ImGui::TreePop();
	}

	ImGui::End();
#endif // _DEBUG
}
