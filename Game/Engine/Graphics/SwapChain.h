#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <wrl.h>

#include <cstdint>

class ColorBuffer;

class SwapChain {
public:
	static const uint32_t kNumBuffers = 3;

	void Create(HWND hWnd);
	void Present();

	ColorBuffer& GetColorBuffer() { return *buffers_[currentBufferIndex_]; }
	const ColorBuffer& GetColorBuffer()const { return *buffers_[currentBufferIndex_]; }
	uint32_t GetBufferIndex() { return currentBufferIndex_; }
private:
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	std::unique_ptr<ColorBuffer> buffers_[kNumBuffers];
	uint32_t currentBufferIndex_ = 0;
	int32_t refreshRate_ = 0;
};