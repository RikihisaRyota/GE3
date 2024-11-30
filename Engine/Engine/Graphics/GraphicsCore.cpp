#include "GraphicsCore.h"

#include <dxgi1_6.h>

#include <algorithm>
#include <cassert>
#include <format>
#include <vector>

#include "../ConvertString/ConvertString.h"
#include "SamplerManager.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

GraphicsCore* GraphicsCore::GetInstance() {
	static GraphicsCore instance;
	return &instance;
}

void GraphicsCore::Initialize() {
	CreateDevice();

	int32_t numDescriptorsTable[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
	numDescriptorsTable[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = kNumRTVs;
	numDescriptorsTable[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = kNumDSVs;
	numDescriptorsTable[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = kNumSRVs;
	numDescriptorsTable[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = kNumSamplers;

	for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
		descriptorHeaps_[i].Create(D3D12_DESCRIPTOR_HEAP_TYPE(i), numDescriptorsTable[i]);
	}

	commandListManager_.Create();
	for (int i = 0; i < LinearAllocatorType::Type::kNumAllocatorTypes; ++i) {
		linearAllocatorPagePools_[i].Initialize((LinearAllocatorType(static_cast<LinearAllocatorType::Type>(i))));
	}

	SamplerManager::Initialize();
}

void GraphicsCore::Shutdown() {
	commandListManager_.Shutdown();
	for (int i = 0; i < LinearAllocatorType::Type::kNumAllocatorTypes; ++i) {
		linearAllocatorPagePools_[i].Finalize();
	}
}

DescriptorHandle GraphicsCore::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	return descriptorHeaps_[type].Allocate();
}

CommandQueue& GraphicsCore::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) {
	switch (type) {
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return commandListManager_.GetComputeQueue();
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return commandListManager_.GetCopyQueue();
	}
	return commandListManager_.GetGraphicsQueue();
}

void GraphicsCore::CreateDevice() {
	HRESULT hr = S_FALSE;
#ifdef _DEBUG
	ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバックレイヤーを有効化する
		debugController->EnableDebugLayer();
		//更にGPU側でもチェックを行うようにする
	//	debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // _DEBUG
	// DXGIファクトリーの生成
	ComPtr<IDXGIFactory7> factory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	assert(SUCCEEDED(hr));
	// アダプターの列挙用
	std::vector<ComPtr<IDXGIAdapter4>> adapters;
	// ここに特定の名前を持つアダプターオブジェクトが入る
	ComPtr<IDXGIAdapter4> tmpAdapter;
	// パフォーマンスが高いものから順に、全てのアダプターを列挙する
	for (UINT i = 0; factory->EnumAdapterByGpuPreference(
		i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&tmpAdapter)) !=
		DXGI_ERROR_NOT_FOUND;
		i++) {
		// 動的配列に追加する
		adapters.push_back(tmpAdapter);
	}
	// 対応レベルの配列
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	// ハードウェアアダプタを優先的に処理
	std::stable_sort(
		adapters.begin(), adapters.end(),
		[](const ComPtr<IDXGIAdapter4>& lhs, const ComPtr<IDXGIAdapter4>& rhs) {
			DXGI_ADAPTER_DESC3 lhsDesc;
			lhs->GetDesc3(&lhsDesc); // アダプターの情報を取得
			DXGI_ADAPTER_DESC3 rhsDesc;
			rhs->GetDesc3(&rhsDesc); // アダプターの情報を取得
			return (lhsDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) <
				(rhsDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE);
		});
	hr = S_FALSE;
	D3D_FEATURE_LEVEL featureLevel;
	for (int i = 0; i < adapters.size(); i++) {
		// デバイスを生成
		for (int levelIndex = 0; levelIndex < _countof(levels); levelIndex++) {
			hr = D3D12CreateDevice(adapters[i].Get(), levels[levelIndex], IID_PPV_ARGS(&device_));
			if (SUCCEEDED(hr)) {
				// デバイスを生成できた時点でループを抜ける
				featureLevel = levels[levelIndex];
				break;
			}
		}
		// このアダプタで生成できてたら完了
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	assert(!!device_);
	assert(SUCCEEDED(hr));

	Log("Complete create D3D12Device!!\n");//初期化完了のログをだす
#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windows11でのDXGIデバックレイヤーの相互作用バグによるエラーメッセージ
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE,
			D3D12_MESSAGE_ID_GPU_BASED_VALIDATION_RESOURCE_STATE_IMPRECISE,
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
		//解放
		infoQueue->Release();
	}
#endif
}
