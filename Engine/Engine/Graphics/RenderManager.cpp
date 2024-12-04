#include "RenderManager.h"

#include <d3dx12.h>

#include "Engine/Graphics/Helper.h"
#include "Engine/ShderCompiler/ShaderCompiler.h"

#include "../../Engine/WinApp/WinApp.h"
#include "../Imgui/ImGuiManager.h"

RenderManager* RenderManager::GetInstance() {
	static RenderManager instance;
	return &instance;
}

void RenderManager::Initialize() {
	// 描画デバイスを初期化
	graphicsCore_ = GraphicsCore::GetInstance();
	graphicsCore_->Initialize();

	// スワップチェーンを初期化
	auto window = WinApp::GetInstance();
	swapChain_.Create(window->GetHwnd());

	// コマンドリストの初期化
	commandContext_.Create();
	//commandContext_.Close();

	// メインとなるバッファを初期化
	uint32_t targetSwapChainBufferIndex = (swapChain_.GetBackBufferIndex() + 1) % SwapChain::kNumBuffers;
	mainColorBufferFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
	mainDepthBufferFormat_ = DXGI_FORMAT_D32_FLOAT;
	auto& swapChainBuffer = swapChain_.GetColorBuffer(targetSwapChainBufferIndex);
	Color clearColor = { 0.0f,0.0f,0.0f,1.0f };
	for (uint32_t i = 0; i < SwapChain::kNumBuffers; i++) {
		swapChain_.GetColorBuffer(i).SetClearColor(clearColor);
	}
	mainColorBuffer_.SetClearColor(clearColor);

	mainColorBuffer_.Create(L"mainColorBuffer", swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight(), mainColorBufferFormat_);

	mainDepthBuffer_.Create(L"mainDepthBuffer", swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight(), mainDepthBufferFormat_);


	// swapchainに送るときのパイプライン
	{
		{
			CD3DX12_DESCRIPTOR_RANGE textureRange[1]{};
			textureRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

			CD3DX12_ROOT_PARAMETER rootParameters[1]{};
			rootParameters[0].InitAsDescriptorTable(_countof(textureRange), textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

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

			rootSignature_.Create(L"toSwapchainRootSigunature", desc);
		}

		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};

			desc.pRootSignature = rootSignature_;

			D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
			inputLayoutDesc.pInputElementDescs = nullptr;
			inputLayoutDesc.NumElements = 0;
			desc.InputLayout = inputLayoutDesc;

			auto vs = ShaderCompiler::Compile(L"Resources/Shaders/Fullscreen.VS.hlsl", L"vs_6_0");
			auto ps = ShaderCompiler::Compile(L"Resources/Shaders/Fullscreen.PS.hlsl", L"ps_6_0");
			desc.VS = CD3DX12_SHADER_BYTECODE(vs->GetBufferPointer(), vs->GetBufferSize());
			desc.PS = CD3DX12_SHADER_BYTECODE(ps->GetBufferPointer(), ps->GetBufferSize());
			desc.BlendState = Helper::BlendDisable;
			desc.DepthStencilState = Helper::DepthStateRead;
			desc.RasterizerState = Helper::RasterizerNoCull;
			desc.NumRenderTargets = 1;
			desc.RTVFormats[0] = mainColorBufferFormat_;
			desc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
			desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			desc.SampleDesc.Count = 1;
			pipelineState_.Create(L"toSwapchainPipeLine", desc);
		}
	}

	postEffect_.Initialize(mainColorBuffer_);

	outLine_.Initialize(mainColorBuffer_);

	gaussianFilter_.Initialize(mainColorBuffer_);

	radialBlur_.Initialize(mainColorBuffer_);

	dissolve_.Initialize(mainColorBuffer_);

	hsvFilter_.Initialize(mainColorBuffer_);

	bloom_.Initialize(mainColorBuffer_);
	// ImGUi初期化

	ImGuiManager::GetInstance()->Initialize(window->GetHwnd(), swapChain_.GetColorBuffer(targetSwapChainBufferIndex).GetFormat());
}

void RenderManager::Reset() {
	auto imguiManager = ImGuiManager::GetInstance();
	imguiManager->NewFrame();

	auto& commandContext = commandContext_;
	//commandContext.Reset();
}

void RenderManager::BeginRender() {
	auto& commandContext = commandContext_;

	commandContext.StartFrame();

	SetRenderTarget(mainColorBuffer_, mainDepthBuffer_);
	commandContext.ClearColor(mainColorBuffer_);
	commandContext.ClearDepth(mainDepthBuffer_);
	commandContext.SetViewportAndScissorRect(0, 0, mainColorBuffer_.GetWidth(), mainColorBuffer_.GetHeight());

}

void RenderManager::BeginDraw() {
	auto& commandContext = commandContext_;
	commandContext.BeginDraw();
}

void RenderManager::EndRender(const ViewProjection& viewProjection) {
	auto& commandContext = commandContext_;
	uint32_t targetSwapChainBufferIndex = (swapChain_.GetBackBufferIndex() + 1) % SwapChain::kNumBuffers;
	auto& swapChainColorBuffer = swapChain_.GetColorBuffer(targetSwapChainBufferIndex);

	//outLine_.Render(commandContext, mainColorBuffer_, mainDepthBuffer_, viewProjection);
	//postEffect_.Render(commandContext,mainColorBuffer_ );
	//gaussianFilter_.Render(commandContext, mainColorBuffer_);
	//radialBlur_.Render(commandContext, mainColorBuffer_);
	//dissolve_.Render(commandContext, mainColorBuffer_);
	//hsvFilter_.Render(commandContext, mainColorBuffer_);
	bloom_.Render(commandContext, mainColorBuffer_);

	commandContext.TransitionResource(QueueType::Type::DIRECT, swapChainColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.SetRenderTarget(swapChainColorBuffer.GetRTV());
	commandContext.ClearColor(swapChainColorBuffer);
	commandContext.SetViewportAndScissorRect(0, 0, swapChainColorBuffer.GetWidth(), swapChainColorBuffer.GetHeight());

	commandContext.SetGraphicsRootSignature(rootSignature_);
	commandContext.SetPipelineState(QueueType::Type::DIRECT, pipelineState_);
	commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandContext.TransitionResource(QueueType::Type::DIRECT, mainColorBuffer_, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandContext.SetGraphicsDescriptorTable(0, mainColorBuffer_.GetSRV());
	commandContext.Draw(3);

	// ImGuiを描画
	auto imguiManager = ImGuiManager::GetInstance();
	imguiManager->Render(commandContext);

	commandContext.TransitionResource(QueueType::Type::DIRECT, swapChainColorBuffer, D3D12_RESOURCE_STATE_PRESENT);

	swapChain_.Present();

	auto& commandQueue = GraphicsCore::GetInstance()->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);

	commandContext.Close();
	commandContext.EndFrame();

	imguiManager->NewFrame();
}

void RenderManager::Shutdown() {
	graphicsCore_->Shutdown();
	ImGuiManager::GetInstance()->Shutdown();
}

void RenderManager::SetRenderTarget(ColorBuffer& target) {
	auto& commandContext = commandContext_;

	commandContext.TransitionResource(QueueType::Type::DIRECT, target, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.SetRenderTarget(target.GetRTV());

}

void RenderManager::SetRenderTarget(ColorBuffer& target, DepthBuffer& depth) {
	auto& commandContext = commandContext_;

	commandContext.TransitionResource(QueueType::Type::DIRECT, target, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.TransitionResource(QueueType::Type::DIRECT, depth, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	commandContext.SetRenderTarget(target.GetRTV(), depth.GetDSV());
}
