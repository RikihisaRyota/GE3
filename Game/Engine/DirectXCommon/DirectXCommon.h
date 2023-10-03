#include <d3d12.h>
#include <dxgi1_6.h>
#include "../../../Externals/DirectXTex/d3dx12.h"

#include <vector>

class WinApp;
class DirectXCommon {
public:// 静的メンバ関数
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static DirectXCommon* GetInstance();

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(WinApp* winApp, int32_t backBufferWidth,int32_t backBufferHeight);

	/// <summary>
	/// 描画前処理
	/// </summary>
	void PreDraw();

	/// <summary>
	/// 描画後処理
	/// </summary>
	void PostDraw();

	/// <summary>
	/// レンダーターゲットのクリア
	/// </summary>
	void ClearRenderTarget(
	);

	/// <summary>
	/// 深度バッファのクリア
	 /// </summary>
	void ClearDepthBuffer();

	/// <summary>
	/// デバイスの取得
	/// </summary>
	/// <returns>デバイス</returns>
	ID3D12Device* GetDevice() const { return device_.Get(); }

	/// <summary>
	/// コマンドリストの取得
	/// </summary>
	/// <returns>コマンドリスト</returns>
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

	/// <summary>
	/// バックバッファの数
	/// </summary>
	/// <returns>バックバッファ</returns>
	size_t GetBackBufferCount()const { return backBuffers_.size(); }

	ID3D12Resource* GetBackBuff()const { return backBuffers_[0].Get(); }

	/// <summary>
	/// RTV
	/// </summary>
	/// <returns></returns>
	ID3D12DescriptorHeap* GetRTVDescriptorHeap() { return rtvDescriptorHeap_.Get(); }

	/// <summary>
	/// DSV
	/// </summary>
	/// <returns></returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle() { return dpsHandle_; }

	/// <summary>
	/// リリース
	/// </summary>
	void Release();

private:// メンバ関数

	/// <summary>
	/// DXGIデバイス初期化
	/// </summary>
	void InitializeDXGIDevice();

	/// <summary>
	/// コマンド関連初期化
	/// </summary>
	void InitializeCommand();

	/// <summary>
	/// スワップチェーンの生成
	/// </summary>
	void CreateSwapChain();

	/// <summary>
	/// レンダーターゲット生成
	/// </summary>
	void CreateRenderTatgets();

	/// <summary>
	///  深度バッファ生成
	/// </summary>
	void CreateDepthBuffer();

	/// <summary>
	/// フェンス生成
	/// </summary>
	void CreateFence();

private: // メンバ関数
	DirectXCommon() = default;
	~DirectXCommon() = default;
	DirectXCommon(const DirectXCommon&) = delete;
	const DirectXCommon& operator=(const DirectXCommon&) = delete;

	/// <summary>
	/// CreateDepthStencilTexture関数
	/// </summary>
	/// <param name="device"></param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <returns></returns>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(int32_t width, int32_t height);

	/// <summary>
	/// CreateDescriptorHeap
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
private:// メンバ変数
	// ウィンドウズアプリケーション管理
	WinApp* winApp_;

	// Direct3D関連
	// DXGI関連
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	// コマンド関連
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	// スワップチェーン関連
	int32_t backBufferWidth_ = 0;
	int32_t backBufferHeight_ = 0;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	// レンダーターゲット関連
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> backBuffers_;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle_;
	// 深度バッファ関連
	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	D3D12_CPU_DESCRIPTOR_HANDLE dpsHandle_;
	// 描画関連
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	UINT64 fenceValue_ = 0;

};