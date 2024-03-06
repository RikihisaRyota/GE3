#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

#include "DescriptorHeap.h"
#include "CommandQueue.h"

class GraphicsCore {
public: 
	static GraphicsCore* GetInstance();

	static const uint32_t kNumRTVs = 16;
	static const uint32_t kNumDSVs = 2;
	static const uint32_t kNumSRVs = 256;
	static const uint32_t kNumSamplers = 16;

	void Initialize();
	void Shutdown();

	DescriptorHandle AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type);

	CommandQueue& GetCommandQueue() {return commandQueue_;}
	ID3D12Device* GetDevice() const { return device_.Get(); }
	DescriptorHeap& GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) {return descriptorHeaps_[type];}
private:

	GraphicsCore() = default;
	GraphicsCore(const GraphicsCore&) = delete;
	GraphicsCore& operator=(const GraphicsCore&) = delete;

	void CreateDevice();

	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	CommandQueue commandQueue_;
	DescriptorHeap descriptorHeaps_[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};

