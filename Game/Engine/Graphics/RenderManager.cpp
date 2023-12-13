#include "RenderManager.h"

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
	for (auto& commandContext : commandContexts_) {
		commandContext.Create();
		commandContext.Close();
	}

	// メインとなるバッファを初期化
	mainColorBufferFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;
	mainDepthBufferFormat_ = DXGI_FORMAT_D32_FLOAT;
	auto& swapChainBuffer = swapChain_.GetColorBuffer();
	Color clearColor = { 0.3f,0.1f,0.3f,0.0f };
	swapChain_.GetColorBuffer().SetClearColor(clearColor);
	mainColorBuffer_.SetClearColor(clearColor);
	mainColorBuffer_.Create(L"mainColorBuffer", swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight(), mainColorBufferFormat_);


	mainDepthBuffer_.Create(L"mainDepthBuffer", swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight(), mainDepthBufferFormat_);
	
	// ImGUi初期化
	ImGuiManager::GetInstance()->Initialize(window->GetHwnd(),swapChain_.GetColorBuffer().GetFormat());
}

void RenderManager::Reset() {
	auto imguiManager = ImGuiManager::GetInstance();
	imguiManager->NewFrame();

	auto& commandContext = commandContexts_[swapChain_.GetBufferIndex()];
	commandContext.Reset();
}

void RenderManager::BeginRender() {
	auto& commandContext = commandContexts_[swapChain_.GetBufferIndex()];
	auto& swapChainColorBuffer = swapChain_.GetColorBuffer();

	commandContext.TransitionResource(swapChainColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.TransitionResource(mainDepthBuffer_, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	commandContext.SetRenderTarget(swapChainColorBuffer.GetRTV(), mainDepthBuffer_.GetDSV());
	Color clearColor = { 0.1f,0.25f,0.5f,1.0f };
	swapChainColorBuffer.SetClearColor(clearColor);
	commandContext.ClearColor(swapChainColorBuffer);
	commandContext.ClearDepth(mainDepthBuffer_);
	commandContext.SetViewportAndScissorRect(0, 0, swapChainColorBuffer.GetWidth(), swapChainColorBuffer.GetHeight());
}

void RenderManager::EndRender() {
	auto& commandContext = commandContexts_[swapChain_.GetBufferIndex()];
	auto& swapChainColorBuffer = swapChain_.GetColorBuffer();
	// ImGuiを描画
	auto imguiManager = ImGuiManager::GetInstance();
	imguiManager->Render(commandContext);

	commandContext.TransitionResource(swapChainColorBuffer, D3D12_RESOURCE_STATE_PRESENT);
	commandContext.Close();
	CommandQueue& commandQueue = graphicsCore_->GetCommandQueue();
	commandQueue.WaitForGPU();
	commandQueue.Execute(commandContext);
	swapChain_.Present();
	commandQueue.Signal();
}

void RenderManager::Shutdown() {
	graphicsCore_->Shutdown();
	ImGuiManager::GetInstance()->Shutdown();
}
