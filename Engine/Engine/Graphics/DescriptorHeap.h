#pragma once
/**
 * @file DescriptorHeap.h
 * @brief ヒープの管理
 */
#include <d3d12.h>
#include <wrl.h>

#include <cstdint>

#include "DescriptorHandle.h"

class DescriptorHeap {
public:
	void Create(D3D12_DESCRIPTOR_HEAP_TYPE type,uint32_t numDescriptors);
	DescriptorHandle Allocate();
	DescriptorHandle GetStartDescriptorHandle() { return firstDescriptors_; }
	operator ID3D12DescriptorHeap* () const { return descriptorHeap_.Get(); }
	uint32_t GetFreeDescriptors() { return numFreeDescriptors_ - 1; }

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	DescriptorHandle firstDescriptors_;
	uint32_t descriptorSize_;
	uint32_t numDescriptors_;
	uint32_t numFreeDescriptors_;
};