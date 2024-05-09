#include "DrawLine.h"

#include "Engine/ShderCompiler/ShaderCompiler.h"
#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/SamplerManager.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Graphics/CommandContext.h"

#include <d3dx12.h>

namespace Rootsignature  {
	enum Parameter {
		kViewprojection,

		kCount,
	};
}

DrawLine* DrawLine::GetInstance() {
	static DrawLine drawLine;
	return &drawLine;
}

void DrawLine::Initialize() {
	CreateRootSignature();
	CreatePipelineState();
	CreateVertexBuffer();
	Reset();
}

void DrawLine::Draw(CommandContext& commandContext, const ViewProjection& viewProjection) {
	if (!vertices_.empty()) {
		memset(vertBuff_.GetCPUData(), 0, vertBuff_.GetBufferSize());

		vertBuff_.Copy(vertices_.data(), vertices_.size() * sizeof(vertices_[0]));
		
		commandContext.SetGraphicsRootSignature(*rootSignature_);
		commandContext.SetPipelineState(*pipelineState_);

		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		commandContext.SetVertexBuffer(0, vbView_);

		commandContext.SetGraphicsConstantBuffer(0,viewProjection.constBuff_.GetGPUVirtualAddress());

		commandContext.Draw(static_cast<UINT>(vertices_.size()));
		
		Reset();
	}
}

void DrawLine::Reset() {
	drawCount_ = 0;
	vertices_.clear();
}

void DrawLine::SetLine(const Vector3& start, const Vector3& end, const Vector4& color) {
	vertices_.emplace_back(Vertex({ start.x,start.y,start.z }, { color }));
	vertices_.emplace_back(Vertex({ end.x,end.y,end.z}, { color }));
}

void DrawLine::SetLine(const Vector3& line, const Vector4& color) {
	vertices_.emplace_back(Vertex({ line.x,line.y,line.z }, { color }));
}

void DrawLine::CreateRootSignature() {
	rootSignature_ = std::make_unique<RootSignature>();

	CD3DX12_ROOT_PARAMETER rootParameters[Rootsignature::Parameter::kCount]{};
	rootParameters[Rootsignature::Parameter::kViewprojection].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	
	//CD3DX12_STATIC_SAMPLER_DESC samplerDesc
	//	= CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);


	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.pParameters = rootParameters;
	desc.NumParameters = _countof(rootParameters);
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//desc.NumStaticSamplers = 1;
	//desc.pStaticSamplers = &samplerDesc;

	rootSignature_->Create(L"DrawLineRootSignature",desc);
}

void DrawLine::CreatePipelineState() {
	pipelineState_ = std::make_unique<PipelineState>();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

	desc.pRootSignature = *rootSignature_;

	D3D12_INPUT_ELEMENT_DESC inputElements[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElements;
	inputLayoutDesc.NumElements = _countof(inputElements);
	desc.InputLayout = inputLayoutDesc;

	auto vs = ShaderCompiler::Compile(L"Resources/Shaders/DrawLine/DrawLine.VS.hlsl", L"vs_6_0");
	auto ps = ShaderCompiler::Compile(L"Resources/Shaders/DrawLine/DrawLine.PS.hlsl", L"ps_6_0");
	desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
	desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
	desc.BlendState = Helper::BlendDisable;
	desc.DepthStencilState = Helper::DepthStateDisabled;
	desc.RasterizerState = Helper::RasterizerNoCull;
	desc.NumRenderTargets = 1;
	desc.RTVFormats[0] = RenderManager::GetInstance()->GetRenderTargetFormat();
	desc.DSVFormat = RenderManager::GetInstance()->GetDepthFormat();
	desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	desc.SampleDesc.Count = 1;
	pipelineState_->Create(L"DrawLine PSO", desc);
}

void DrawLine::CreateVertexBuffer() {
#pragma region 頂点バッファ
	// 頂点データのサイズ
	UINT sizeVB = static_cast<UINT>(sizeof(Vertex) * kMaxLineCount);
	vertBuff_ .Create(L"DrawLineVertexBuffer",sizeVB);
	// 頂点バッファビューの作成
	vbView_.BufferLocation = vertBuff_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = sizeVB;
	vbView_.StrideInBytes = sizeof(vertices_[0]);
#pragma endregion 頂点バッファ
}