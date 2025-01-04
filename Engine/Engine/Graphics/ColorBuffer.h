#pragma once
/**
 * @file ColorBuffer.h
 * @brief 主にレンダーターゲットに使用
 */
#include "PixelBuffer.h"
#include "Color.h"

#include "DescriptorHandle.h"

class ColorBuffer :public PixelBuffer {
public:
	// SwapChain用
	void CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* resource);
	// 生成
	void Create(const std::wstring& name, uint32_t width, uint32_t height, DXGI_FORMAT format);

	// setter / getter
	void SetClearColor(Color ClearColor) { clearColor_ = ClearColor; }
	Color GetClearColor()const { return clearColor_; }

	// GetView
	const DescriptorHandle& GetSRV() const { return srvHandle_; }
	const DescriptorHandle& GetRTV() const { return rtvHandle_; }

private:
	void CreateViews();

	Color clearColor_;
	DescriptorHandle srvHandle_;
	DescriptorHandle rtvHandle_;
};