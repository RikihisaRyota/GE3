#include "GPUParticle.h"

#include <d3dx12.h>

#include "Engine/Graphics/Helper.h"
#include "Engine/Graphics/GraphicsCore.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"

std::vector<uint32_t> test;

void GPUParticle::Initialize() {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	graphicsPipelineState_ = std::make_unique<PipelineState>();
	computePipelineState_ = std::make_unique<PipelineState>();
	graphicsRootSignature_ = std::make_unique<RootSignature>();
	computeRootSignature_ = std::make_unique<RootSignature>();
	for (size_t i = 0; i < kNumThread; i++) {
		test.emplace_back(uint32_t());
		test.back() = 0;
	}

	// グラフィックスルートシグネイチャ
	{
		//CD3DX12_DESCRIPTOR_RANGE ranges[1]{};
		//ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		//CD3DX12_DESCRIPTOR_RANGE samplerRanges[1]{};
		//samplerRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
		//
		//CD3DX12_ROOT_PARAMETER rootParameters[4]{};
		//rootParameters[0].InitAsConstantBufferView(0);
		//rootParameters[1].InitAsConstantBufferView(1);
		//rootParameters[2].InitAsDescriptorTable(_countof(ranges), ranges);
		//rootParameters[3].InitAsDescriptorTable(_countof(samplerRanges), samplerRanges);
		//
		//D3D12_ROOT_SIGNATURE_DESC desc{};
		//desc.pParameters = rootParameters;
		//desc.NumParameters = _countof(rootParameters);
		//desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		//
		//graphicsRootSignature_->Create(L"GPUParticle RootSignature", desc);
	}
	// グラフィックスパイプライン
	{
		//D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		//
		//desc.pRootSignature = *graphicsRootSignature_;
		//
		//D3D12_INPUT_ELEMENT_DESC inputElements[] = {
		//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		//};
		//D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
		//inputLayoutDesc.pInputElementDescs = inputElements;
		//inputLayoutDesc.NumElements = _countof(inputElements);
		//desc.InputLayout = inputLayoutDesc;
		//
		//auto vs = ShaderCompiler::Compile(L"Resources/Shader/GPUParticle.VS.hlsl", L"vs_6_0");
		//auto ps = ShaderCompiler::Compile(L"Resources/Shader/GPUParticle.PS.hlsl", L"ps_6_0");
		//desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
		//desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
		//desc.BlendState = Helper::BlendDisable;
		//desc.DepthStencilState = Helper::DepthStateReadWrite;
		//desc.RasterizerState = Helper::RasterizerDefault;
		//desc.NumRenderTargets = 1;
		//desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		//desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		//desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		//desc.SampleDesc.Count = 1;
		//graphicsPipelineState_->Create(L"GPUParticle PSO", desc);
	}
	// コンピュートシグネイチャ
	{
		CD3DX12_ROOT_PARAMETER rootParameters[1]{};
		rootParameters[0].InitAsUnorderedAccessView(0);

		D3D12_ROOT_SIGNATURE_DESC desc{};
		desc.pParameters = rootParameters;
		desc.NumParameters = _countof(rootParameters);

		computeRootSignature_->Create(L"GPUParticle RootSignature", desc);
	}
	// コンピュートパイプライン
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = *computeRootSignature_;
		auto cs = ShaderCompiler::Compile(L"Game/Resources/Shaders/TestCS.hlsl", L"cs_6_0");
		desc.CS = CD3DX12_SHADER_BYTECODE(cs->GetBufferPointer(), cs->GetBufferSize());
		computePipelineState_->Create(L"GPUParticle CPSO", desc);
	}
	// RWStructuredBuffer
	{
		auto desc = CD3DX12_RESOURCE_DESC::Buffer(
			UINT64(sizeof(uint32_t) * kNumThread),
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
	// UAVの生成
	{
		uavHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_UNORDERED_ACCESS_VIEW_DESC desc{};
		desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		desc.Buffer.NumElements = kNumThread;
		desc.Buffer.StructureByteStride = sizeof(uint32_t);
		device->CreateUnorderedAccessView(
			rwStructuredBuffer_,
			nullptr,
			&desc,
			uavHandle_
		);
	}
	// マップ
	{
		/*rwStructuredBuffer_->Map(0, 0, reinterpret_cast<void**>(&updateParticle_));
		memset(updateParticle_, 0, sizeof(uint32_t) * kNumThread);*/
	}
}

void GPUParticle::Update(CommandContext& commandContext) {
	/*commandContext.operator ID3D12GraphicsCommandList* ()->ClearUnorderedAccessViewUint(
		uavHandle_,uavHandle_,rwStructuredBuffer_,);*/
	commandContext.TransitionResourse(rwStructuredBuffer_, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandContext.SetPipelineState(*computePipelineState_);
	commandContext.SetComputeRootSignature(*computeRootSignature_);
	commandContext.SetComputeUAV(0, rwStructuredBuffer_->GetGPUVirtualAddress());
	commandContext.Dispatch(16,1,1);

}

void GPUParticle::Render(CommandContext& commandContext) {

}
