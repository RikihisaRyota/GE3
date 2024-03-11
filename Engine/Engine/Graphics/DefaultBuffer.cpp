#include "DefaultBuffer.h"

#include <d3dx12.h>

#include <assert.h>

#include "GraphicsCore.h"

DefaultBuffer::~DefaultBuffer() {
	Destroy();
}

void DefaultBuffer::Create(const std::wstring& name, size_t bufferSize, D3D12_RESOURCE_FLAGS flags) {
	auto device = GraphicsCore::GetInstance()->GetDevice();

	Destroy();

	auto desc = CD3DX12_RESOURCE_DESC::Buffer(UINT64(bufferSize), flags);
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	auto result = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(resource_.GetAddressOf())
	);
	assert(SUCCEEDED(result));

	state_ = D3D12_RESOURCE_STATE_COMMON;

	resource_->SetName(name.c_str());
}

void DefaultBuffer::Destroy() {
	if (resource_) {
		resource_->Unmap(0, nullptr);
		resource_.Reset();
	}
}
