#pragma once
/**
 * @file SwapChain.h
 * @brief SwapChain用
 */
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <wrl.h>

#include <cstdint>

class ColorBuffer;
class SwapChain {
public:
	static const uint32_t kNumBuffers = 3;
	// 生成
	void Create(HWND hWnd);
	// Present
	void Present();

	// Getter
	ColorBuffer& GetColorBuffer(uint32_t index) { return *buffers_[index]; }
	uint32_t GetBackBufferIndex() { return swapChain_->GetCurrentBackBufferIndex(); }

private:
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	std::unique_ptr<ColorBuffer> buffers_[kNumBuffers];
	int32_t refreshRate_ = 0;
};