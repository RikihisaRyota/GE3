#include "ReadBackBuffer.h"

#include <assert.h>

#include <d3dx12.h>

#include "GraphicsCore.h"

ReadBackBuffer::~ReadBackBuffer() {
	Destroy();
}

void ReadBackBuffer::Create(const std::wstring& name, size_t bufferSize) {
	auto device = GraphicsCore::GetInstance()->GetDevice();

	Destroy();

	auto desc = CD3DX12_RESOURCE_DESC::Buffer(UINT64(bufferSize));
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);

	auto result = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(resource_.GetAddressOf())
	);
	assert(SUCCEEDED(result));

	state_ = D3D12_RESOURCE_STATE_COPY_DEST;
	bufferSize_ = bufferSize;

	resource_->Map(0, nullptr, &cpuData_);

	resource_->SetName(name.c_str());
}

void ReadBackBuffer::Create(const std::wstring& name, size_t numElements, size_t elementSize) {
	Create(name, numElements * elementSize);
}

void ReadBackBuffer::ResetBuffer() {
	memset(cpuData_, 0, bufferSize_);
}

void ReadBackBuffer::Destroy() {
	if (cpuData_ && resource_) {
		resource_->Unmap(0, nullptr);
		resource_.Reset();
		cpuData_ = nullptr;
	}
}
