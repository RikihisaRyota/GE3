#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

#include "DescriptorHeap.h"
#include "CommandListManager.h"
#include "LinerAllocator.h"

class GraphicsCore {
public: 
	static GraphicsCore* GetInstance();

	static const uint32_t kNumRTVs = 32;
	static const uint32_t kNumDSVs = 2;
	static const uint32_t kNumSRVs = 256;
	static const uint32_t kNumSamplers = 16;

	void Initialize();
	void Shutdown();

	DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);

	CommandQueue& GetCommandQueue(D3D12_COMMAND_LIST_TYPE type);
	ID3D12Device* GetDevice() const { return device_.Get(); }
	DescriptorHeap& GetDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE& type) {return descriptorHeaps_[type];}
	LinearAllocatorPagePool& GetLinearAllocatorPagePool(const LinearAllocatorType& type) { return linearAllocatorPagePools_[type.type]; }
private:

	GraphicsCore() = default;
	GraphicsCore(const GraphicsCore&) = delete;
	GraphicsCore& operator=(const GraphicsCore&) = delete;

	void CreateDevice();

	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	D3D12_COMMAND_LIST_TYPE type_;
	CommandListManager commandListManager_;
	DescriptorHeap descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

	LinearAllocatorPagePool linearAllocatorPagePools_[LinearAllocatorType::kNumAllocatorTypes];
};

