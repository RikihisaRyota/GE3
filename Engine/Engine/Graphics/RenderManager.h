#pragma once
/**
 * @file RenderManager.h
 * @brief 描画に関する物をまとめている
 */
#include "CommandContext.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GraphicsCore.h"
#include "SwapChain.h"
#include "PostEffect.h"
#include "Outline.h"
#include "GaussianFilter.h"
#include "RadialBlur.h"
#include "Dissolve.h"
#include "HSVFilter.h"
#include "Bloom.h"

struct ViewProjection;
class RenderManager {

public:
	static RenderManager* GetInstance();

	void Initialize();
	void Reset();
	void BeginRender();
	void BeginDraw();
	void EndRender(const ViewProjection& viewProjection);
	void Shutdown();

	PostEffect& GetPostEffect() { return postEffect_; }
	Outline& GetOutline() { return  outLine_; }
	GaussianFilter& GetGaussianFilter() { return gaussianFilter_; }
	RadialBlur& GetRadialBlur() { return radialBlur_; }
	Dissolve& GetDissolve() { return dissolve_; }
	HSVFilter& GetHSVFilter() { return hsvFilter_; }
	Bloom& GetBloom() { return bloom_; }

	CommandContext& GetCommandContext() { return commandContext_; }

	ColorBuffer& GetMainColorBuffer() { return mainColorBuffer_; }
	DepthBuffer& GetMainDepthBuffer() { return mainDepthBuffer_; }

	SwapChain& GetSwapChain() { return swapChain_; }
	DXGI_FORMAT GetRenderTargetFormat() { return mainColorBufferFormat_; }
	DXGI_FORMAT GetDepthFormat() { return mainDepthBufferFormat_; }

	void SetRenderTarget(ColorBuffer& target);
	void SetRenderTarget(ColorBuffer& target,DepthBuffer& depth);
private:
	RenderManager() = default;
	RenderManager(const RenderManager&) = delete;
	RenderManager& operator=(const RenderManager&) = delete;

	GraphicsCore* graphicsCore_ = nullptr;
	CommandContext commandContext_;
	SwapChain swapChain_;

	ColorBuffer mainColorBuffer_;
	DepthBuffer mainDepthBuffer_;

	DXGI_FORMAT mainColorBufferFormat_;
	DXGI_FORMAT mainDepthBufferFormat_;

	PostEffect postEffect_;
	Outline outLine_;
	GaussianFilter gaussianFilter_;
	RadialBlur radialBlur_;
	Dissolve dissolve_;
	HSVFilter hsvFilter_;
	Bloom bloom_;

	PipelineState pipelineState_;
	RootSignature rootSignature_;
};