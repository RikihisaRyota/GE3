#pragma once

#include "CommandContext.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GraphicsCore.h"
#include "SwapChain.h"

class RenderManager {

public:
	static RenderManager* GetInstance();
	
	void Initialize();
	void Reset();
	void BeginRender();
	void EndRender();
	void Shutdown();

	CommandContext& GetCommandContext() { return commandContexts_[swapChain_.GetBufferIndex()]; }
	SwapChain& GetSwapChain() { return swapChain_; }
	DXGI_FORMAT GetRenderTargetFormat() { return mainColorBufferFormat_; }
	DXGI_FORMAT GetDepthFormat() { return mainDepthBufferFormat_; }
private:
	RenderManager() = default;
	RenderManager(const RenderManager&) = delete;
	RenderManager& operator=(const RenderManager&) = delete;

	GraphicsCore* graphicsCore_ = nullptr;
	CommandContext commandContexts_[SwapChain::kNumBuffers];
	SwapChain swapChain_;

	ColorBuffer mainColorBuffer_;
	DepthBuffer mainDepthBuffer_;

	DXGI_FORMAT mainColorBufferFormat_;
	DXGI_FORMAT mainDepthBufferFormat_;
};