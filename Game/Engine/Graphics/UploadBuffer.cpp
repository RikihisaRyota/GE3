#include "UploadBuffer.h"

#include <d3dx12.h>

#include <assert.h>

#include "GraphicsCore.h"

UploadBuffer::~UploadBuffer() {
	Destroy();
}

void UploadBuffer::Create(const std::wstring& name, size_t bufferSize) {
	auto device = GraphicsCore::GetInstance()->GetDevice();

	Destroy();

	auto desc = CD3DX12_RESOURCE_DESC::Buffer(UINT64(bufferSize));
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	auto result = device->CreateCommittedResource(
	&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(resource_.GetAddressOf())
	);
	assert(result);
	
	state_ = D3D12_RESOURCE_STATE_GENERIC_READ;
	bufferSize_ = bufferSize;

	resource_->Map(0, nullptr, &cpuData_);

	resource_->SetName(name.c_str());
}

void UploadBuffer::Create(const std::wstring& name, size_t numElements, size_t elementSize) {
	Create(name, numElements * elementSize);
}

void UploadBuffer::Copy(const void* srcData, size_t copySize) const {
	assert(copySize <= bufferSize_);
	memcpy(cpuData_, srcData, copySize);
}

void UploadBuffer::Destroy() {
	if (cpuData_ && resource_) {
		resource_->Unmap(0, nullptr);
		resource_.Reset();
		cpuData_ = nullptr;
	}
}
