#pragma once

#include "PixelBuffer.h"
#include "Color.h"

#include "DescriptorHandle.h"

class ColorBuffer :public PixelBuffer {
public:
	void CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* resource);
	void Create(const std::wstring& name, uint32_t width, uint32_t height, DXGI_FORMAT format);

	void SetClearColor(Color ClearColor) { clearColor_ = ClearColor; }
	Color GetClearColor()const { return clearColor_; }

	const DescriptorHandle& GetSRV() const { return srvHandle_; }
	const DescriptorHandle& GetRTV() const { return rtvHandle_; }

private:
	void CreateViews();

	Color clearColor_;
	DescriptorHandle srvHandle_;
	DescriptorHandle rtvHandle_;
};