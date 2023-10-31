#include "SwapChain.h"

#include <assert.h>

#include "../Graphics/GraphicsCore.h"
#include "ColorBuffer.h"

using namespace Microsoft::WRL;

void SwapChain::Create(HWND hWnd) {
	HRESULT hr = S_FALSE;
	ComPtr<IDXGIFactory7> factory;
	hr = CreateDXGIFactory(IID_PPV_ARGS(factory.GetAddressOf()));

	RECT clientRect{};
	if (!GetClientRect(hWnd, &clientRect)) {
		assert(false);
	}

	HDC hdc = GetDC(hWnd);
	refreshRate_ = GetDeviceCaps(hdc, VREFRESH);
	ReleaseDC(hWnd, hdc);

	auto clientWidth = clientRect.right - clientRect.left;
	auto clientHeight = clientRect.bottom - clientRect.top;

	// スワップチェーンの生成
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = clientWidth;//画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = clientHeight;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//色の形式
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに写したら、中身を破棄
	ComPtr<IDXGISwapChain1> swapChain1;
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = factory->CreateSwapChainForHwnd(
		GraphicsCore::GetInstance()->GetDevice(),
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1);
	assert(SUCCEEDED(hr));

	// swapChain4を得る
	swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain_));
	assert(SUCCEEDED(hr));

	swapChain_->SetMaximumFrameLatency(1);

	for (UINT i = 0; i < kNumBuffers; i++) {
		ComPtr<ID3D12Resource> resource;
		swapChain_->GetBuffer(i, IID_PPV_ARGS(&resource));
		buffers_[i] = std::make_unique<ColorBuffer>();
		buffers_[i]->CreateFromSwapChain(L"SwapChainBuffer"+std::to_wstring(i),resource.Detach());
	}
}

void SwapChain::Present() {
	static constexpr int32_t kThreasholdRefreshRate = 58;
	swapChain_->Present(refreshRate_ < kThreasholdRefreshRate ? 0 : 1, 0);
	currentBufferIndex_ = (currentBufferIndex_ + 1) % kNumBuffers;
}
