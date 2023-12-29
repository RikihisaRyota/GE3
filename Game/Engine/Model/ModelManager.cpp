#include "ModelManager.h"

#include <d3dx12.h>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Texture/TextureManager.h"

std::unique_ptr<RootSignature> ModelManager::rootSignature_;
std::unique_ptr<PipelineState> ModelManager::pipelineState_;

ModelManager* ModelManager::GetInstance() {
	static ModelManager instance;
	return &instance;
}

void ModelManager::CreatePipeline(DXGI_FORMAT rtvFormat, DXGI_FORMAT dsvFormat) {
	pipelineState_ = std::make_unique<PipelineState>();
	rootSignature_ = std::make_unique<RootSignature>();
	{
		CD3DX12_DESCRIPTOR_RANGE range[1]{};
		range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE samplerRanges[1]{};
		samplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[5]{};
		rootParameters[0].InitAsConstantBufferView(0);
		rootParameters[1].InitAsConstantBufferView(1);
		rootParameters[2].InitAsConstantBufferView(2);
		rootParameters[3].InitAsDescriptorTable(_countof(range), range);
		rootParameters[4].InitAsDescriptorTable(_countof(samplerRanges), samplerRanges);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		rootSignature_->Create(L"Model RootSignature", desc);
	}
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *rootSignature_;

		D3D12_INPUT_ELEMENT_DESC inputElements[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		inputLayoutDesc.pInputElementDescs = inputElements;
		inputLayoutDesc.NumElements = _countof(inputElements);
		desc.InputLayout = inputLayoutDesc;

		auto vs = ShaderCompiler::Compile(L"Game/Resources/Shaders/Model.VS.hlsl", L"vs_6_0");
		auto ps = ShaderCompiler::Compile(L"Game/Resources/Shaders/Model.PS.hlsl", L"ps_6_0");

		desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
		desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
		desc.BlendState = Helper::BlendDisable;
		desc.DepthStencilState = Helper::DepthStateReadWrite;
		desc.RasterizerState = Helper::RasterizerDefault;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = rtvFormat;
		desc.DSVFormat = dsvFormat;
		desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		pipelineState_->Create(L"Model PipelineState", desc);
	}
}

void ModelManager::DestroyPipeline() {
	rootSignature_.reset();
	pipelineState_.reset();
}

ModelHandle ModelManager::Load(const std::filesystem::path path) {
	ModelHandle handle;
	// 読み込み済みか探す
	auto iter = std::find_if(models_.begin(), models_.end(), [&](const auto& model) { return model->GetName() == path.stem(); });
	// 読み込み済み
	if (iter != models_.end()) {
		handle.index_ = std::distance(models_.begin(), iter);
		return handle;
	}

	// 最後尾に読み込む
	handle.index_ = models_.size();

	auto model = std::make_unique<Model>();
	model->Create(path);

	models_.emplace_back(std::move(model));
	return handle;
}

void ModelManager::Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection, const ModelHandle& modelHandle, CommandContext& commandContext) {
	commandContext.SetGraphicsRootSignature(*rootSignature_);
	commandContext.SetPipelineState(*pipelineState_);
	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext.SetVertexBuffer(0, models_.at(modelHandle)->GetVBView());
	/*for (auto& modelData : models_.at(modelHandle)->GetMeshData()) {
		commandContext.SetIndexBuffer(modelData->ibView_);
		commandContext.SetGraphicsConstantBuffer(0, worldTransform.constBuff_.get()->GetGPUVirtualAddress());
		commandContext.SetGraphicsConstantBuffer(1, viewProjection.constBuff_.get()->GetGPUVirtualAddress());
		commandContext.SetGraphicsConstantBuffer(2, models_.at(modelHandle)->GetMaterialBuffer().GetGPUVirtualAddress());
		commandContext.SetGraphicsDescriptorTable(3, TextureManager::GetInstance()->GetTexture(models_.at(modelHandle)->GetTextureHandle()).GetSRV());
		commandContext.SetGraphicsDescriptorTable(4, SamplerManager::Anisotropic);
		commandContext.DrawIndexed(modelData->meshes_->indexCount);
	}*/
}
