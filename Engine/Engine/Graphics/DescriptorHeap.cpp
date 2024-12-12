#include "DescriptorHeap.h"

#include <cassert>

#include "GraphicsCore.h"

void DescriptorHeap::Create(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {

	auto device = GraphicsCore::GetInstance()->GetDevice();

	// デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Type = type;
	desc.NumDescriptors = numDescriptors;
	// GPUを識別するためのビットフラグ
	desc.NodeMask = 0;
	// シェーダーから見える必要があれば
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	}
	else {
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	}
	auto hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(descriptorHeap_.ReleaseAndGetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));

	descriptorSize_ = device->GetDescriptorHandleIncrementSize(desc.Type);
	numDescriptors_ = desc.NumDescriptors;
	firstDescriptors_.cpuHandle_ = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	if (desc.Flags == D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE) {
		firstDescriptors_.gpuHandle_ = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	}
	numFreeDescriptors_ = 0;
}

DescriptorHandle DescriptorHeap::Allocate() {
	assert(numFreeDescriptors_ <= numDescriptors_);
	DescriptorHandle allocationHandle{};
	allocationHandle.cpuHandle_ = firstDescriptors_;
	allocationHandle.cpuHandle_.ptr += uint64_t(numFreeDescriptors_) * descriptorSize_;
	if (firstDescriptors_.IsShaderVisible()) {
		allocationHandle.gpuHandle_ = firstDescriptors_;
		allocationHandle.gpuHandle_.ptr += uint64_t(numFreeDescriptors_) * descriptorSize_;
	}
	++numFreeDescriptors_;
	return allocationHandle;
}