#include "SpriteManager.h"

#include <d3dx12.h>

#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"

std::unique_ptr<RootSignature> SpriteManager::rootSignature_;
std::unique_ptr<PipelineState> SpriteManager::pipelineState_;
namespace Parameter {
	enum RootParameter {
		kWorldTransform,
		kMaterial,
		kTexture,
		kSampler,
		kCount,
	};
}

SpriteManager* SpriteManager::GetInstance() {
	static SpriteManager instance;
	return &instance;
}

void SpriteManager::CreatePipeline(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat) {
	pipelineState_ = std::make_unique<PipelineState>();
	rootSignature_ = std::make_unique<RootSignature>();
	{
		CD3DX12_DESCRIPTOR_RANGE range[1]{};
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE samplerRanges[1]{};
		samplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[Parameter::RootParameter::kCount]{};
		rootParameters[Parameter::RootParameter::kWorldTransform].InitAsConstantBufferView(0);
		rootParameters[Parameter::RootParameter::kMaterial].InitAsConstantBufferView(1);
		rootParameters[Parameter::RootParameter::kTexture].InitAsDescriptorTable(_countof(range), range);
		rootParameters[Parameter::RootParameter::kSampler].InitAsDescriptorTable(_countof(samplerRanges), samplerRanges);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		rootSignature_->Create(L"Sprite RootSignature", desc);
	}
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *rootSignature_;

		D3D12_INPUT_ELEMENT_DESC inputElements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElements;
		inputLayoutDesc.NumElements = _countof(inputElements);
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Resources/Shaders/Sprite/Sprite.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Resources/Shaders/Sprite/Sprite.PS.hlsl", L"ps_6_0");

		desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
		desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
		desc.BlendState = Helper::BlendAlpha;
		desc.DepthStencilState = Helper::DepthStateRead;
		desc.RasterizerState = Helper::RasterizerDefault;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = rtvFormat;
		desc.DSVFormat = dsvFormat;
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		pipelineState_->Create(L"Sprite PipelineState", desc);
	}
}

void SpriteManager::DestroyPipeline() {
	rootSignature_.reset();
	pipelineState_.reset();
}

SpriteHandle SpriteManager::Create(
	TextureHandle textureHandle, Vector2 position, Vector2 anchorpoint, Vector4 color, bool isFlipX, bool isFlipY) {
	SpriteHandle handle;
	// 読み込み済みか探す
	auto iter = std::find_if(sprites_.begin(), sprites_.end(), [&](const auto& sprite) { return sprite->GetTextureHandle() == textureHandle; });
	// 読み込み済み
	if (iter != sprites_.end()) {
		handle.index_ = std::distance(sprites_.begin(), iter);
		return handle;
	}

	// 最後尾に読み込む
	handle.index_ = sprites_.size();

	Sprite* sprite = new Sprite();
	sprite= Sprite::Create(textureHandle, position, color,
		anchorpoint, isFlipX, isFlipY);

	sprites_.emplace_back(std::move(sprite));
	return handle;
}

void SpriteManager::Draw(const SpriteHandle& spriteHandle, CommandContext& commandContext) {
	commandContext.SetGraphicsRootSignature(*rootSignature_);
	commandContext.SetPipelineState(QueueType::Type::DIRECT, *pipelineState_);
	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	sprites_.at(spriteHandle)->Draw(commandContext);
}
