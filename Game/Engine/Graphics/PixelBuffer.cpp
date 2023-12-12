#include "PixelBuffer.h"

#include <cassert>

#include "../DirectXTex/d3dx12.h"
#include "GraphicsCore.h"


void PixelBuffer::CreateTextureResource(const std::wstring& name, const D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE clearValue) {
	resource_.Reset();

	auto device = GraphicsCore::GetInstance()->GetDevice();

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = device->CreateCommittedResource(
	&heapProps,D3D12_HEAP_FLAG_NONE,
		&desc,D3D12_RESOURCE_STATE_COMMON,
		&clearValue,IID_PPV_ARGS(resource_.GetAddressOf())
	);

	state_ = D3D12_RESOURCE_STATE_COMMON;

	resource_->SetName(name.c_str());
}

void PixelBuffer::AssociateWithResource(const std::wstring& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES state) {
	assert(resource);

	auto resourceDesc = resource->GetDesc();
	
	resource_.Attach(resource);
	state_ = state;

	width_ = (uint32_t)resourceDesc.Width;
	height_ = resourceDesc.Height;
	arraySize_ = resourceDesc.DepthOrArraySize;
	format_ = resourceDesc.Format;

	resource_->SetName(name.c_str());
}

D3D12_RESOURCE_DESC PixelBuffer::DescribeTex2D(uint32_t width, uint32_t height, uint32_t arraySize, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags) {
	width_ = width;
	height_ = height;
	arraySize_ = arraySize;
	format_ = format;

	return CD3DX12_RESOURCE_DESC::Tex2D(format, UINT64(width), height, UINT16(arraySize), 1, 1, 0, flags);
}
