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
		commandContext.FlushResourceBarriers();
	}

	// メインとなるバッファを初期化
	auto& swapChainBuffer = swapChain_.GetColorBuffer();
	Color clearColor = { 0.3f,0.1f,0.3f,0.0f };
	mainColorBuffer_.SetClearColor(clearColor);
	mainColorBuffer_.Create(L"mainColorBuffer", swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight(), DXGI_FORMAT_R32G32B32A32_FLOAT);


	mainDepthBuffer_.Create(L"mainDepthBuffer", swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight(), DXGI_FORMAT_D32_FLOAT);
	
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

	// メインバッファーをレンダーターゲットに
	commandContext.TransitionResourse(mainColorBuffer_,D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.TransitionResourse(mainDepthBuffer_,D3D12_RESOURCE_STATE_DEPTH_WRITE);
	commandContext.SetRenderTarget(mainColorBuffer_.GetRTV(), mainDepthBuffer_.GetDSV());
	commandContext.ClearColor(mainColorBuffer_);
	commandContext.ClearDepth(mainDepthBuffer_);
	commandContext.SetViewportAndScissorRect(0, 0, mainColorBuffer_.GetWidth(), mainColorBuffer_.GetHeight());
}

void RenderManager::EndRender() {
	auto& commandContext = commandContexts_[swapChain_.GetBufferIndex()];

	// スワップチェーンをレンダーターゲットへ
	auto& swapChainBuffer = swapChain_.GetColorBuffer();
	commandContext.TransitionResourse(swapChainBuffer,D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandContext.SetRenderTarget(swapChainBuffer.GetRTV());
	commandContext.ClearColor(swapChainBuffer);
	commandContext.SetViewportAndScissorRect(0,0,swapChainBuffer.GetWidth(), swapChainBuffer.GetHeight());
	// ImGuiを描画
	auto imguiManager = ImGuiManager::GetInstance();
	imguiManager->Render(commandContext);

	commandContext.TransitionResourse(swapChainBuffer, D3D12_RESOURCE_STATE_PRESENT);
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
