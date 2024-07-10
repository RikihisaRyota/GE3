#include "DefaultBuffer.h"

#include <assert.h>

#include <d3dx12.h>

#include "GraphicsCore.h"

DefaultBuffer::~DefaultBuffer() {
	Destroy();
}

void DefaultBuffer::Create(const std::wstring& name, size_t bufferSize, D3D12_RESOURCE_FLAGS flags) {
	auto device = GraphicsCore::GetInstance()->GetDevice();

	Destroy();

	auto desc = CD3DX12_RESOURCE_DESC::Buffer(UINT64(bufferSize + sizeof(UINT)), flags);
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

	counterOffset_ = AlignForUavCounter(UINT(bufferSize));
}

void DefaultBuffer::CreateUAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc) {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	uavHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateUnorderedAccessView(
		resource_.Get(),
		resource_.Get(),
		&desc,
		uavHandle_);
}

void DefaultBuffer::CreateSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
	auto graphics = GraphicsCore::GetInstance();
	auto device = GraphicsCore::GetInstance()->GetDevice();
	srvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	device->CreateShaderResourceView(
		resource_.Get(),
		&desc,
		srvHandle_
	);
}

void DefaultBuffer::Destroy() {
	if (resource_) {
		resource_.Reset();
	}
}
