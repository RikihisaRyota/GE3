#include "GaussianFilter.h"

#include <d3dx12.h>

#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"

void GaussianFilter::Initialize(const ColorBuffer& target) {
	{
		CD3DX12_DESCRIPTOR_RANGE textureRange[1]{};
		textureRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[RootParameter::kCount]{};
		rootParameters[RootParameter::kTexture].InitAsDescriptorTable(_countof(textureRange), textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

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

		rootSignature_.Create(L"GaussianFilterRootSigunature", desc);
	}
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

		desc.pRootSignature = rootSignature_;

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = nullptr;
		inputLayoutDesc.NumElements = 0;
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Resources/Shaders/Fullscreen.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Resources/Shaders/GaussianFilter/HorizontalGaussianFilter.PS.hlsl", L"ps_6_0");
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
		horizontalPipelineState_.Create(L"HorizontalPipelineState", desc);

		ps = ShaderCompiler::Compile(L"Resources/Shaders/GaussianFilter/VerticalGaussianFilter.PS.hlsl", L"ps_6_0");
		desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());

		verticalPipelineState_.Create(L"VerticalPipelineState", desc);
	}
	{
		originalTexture_.Create(L"OriginalGaussianFilter", target.GetWidth(), target.GetHeight(), target.GetFormat());
		horizontalBuffer_.Create(L"HorizontalBuffer", uint32_t(target.GetWidth() * 0.5f), target.GetHeight(), target.GetFormat());
		verticalBuffer_.Create(L"VerticalBuffer", uint32_t(target.GetWidth() * 0.5f), uint32_t(target.GetHeight() * 0.5f), target.GetFormat());
	}
}

void GaussianFilter::Render(CommandContext& commandContext, ColorBuffer& target) {
	// 水平方向
	commandContext.CopyBuffer(QueueType::Type::DIRECT, originalTexture_, target);
	commandContext.TransitionResource(QueueType::Type::DIRECT, originalTexture_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandContext.TransitionResource(QueueType::Type::DIRECT, horizontalBuffer_, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.SetRenderTarget(horizontalBuffer_.GetRTV());
	commandContext.ClearColor(horizontalBuffer_);
	commandContext.SetViewportAndScissorRect(0, 0, horizontalBuffer_.GetWidth(), horizontalBuffer_.GetHeight());

	commandContext.SetGraphicsRootSignature(rootSignature_);
	commandContext.SetPipelineState(QueueType::Type::DIRECT, horizontalPipelineState_);
	commandContext.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext.SetGraphicsDescriptorTable(0, originalTexture_.GetSRV());
	commandContext.Draw(3);

	commandContext.TransitionResource(QueueType::Type::DIRECT, horizontalBuffer_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandContext.TransitionResource(QueueType::Type::DIRECT, verticalBuffer_, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.ClearColor(verticalBuffer_);
	commandContext.SetRenderTarget(verticalBuffer_.GetRTV());
	commandContext.SetViewportAndScissorRect(0, 0, verticalBuffer_.GetWidth(), verticalBuffer_.GetHeight());

	commandContext.SetGraphicsRootSignature(rootSignature_);
	commandContext.SetPipelineState(QueueType::Type::DIRECT, verticalPipelineState_);
	commandContext.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext.SetGraphicsDescriptorTable(0, horizontalBuffer_.GetSRV());
	commandContext.Draw(3);
}