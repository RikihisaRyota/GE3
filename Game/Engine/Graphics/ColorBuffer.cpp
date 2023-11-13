#include "ColorBuffer.h"

#include "GraphicsCore.h"

void ColorBuffer::CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* resource) {
	AssociateWithResource(Name, resource, D3D12_RESOURCE_STATE_PRESENT);
	auto graphics = GraphicsCore::GetInstance();
	if (rtvHandle_.IsNull()) {
		rtvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	graphics->GetDevice()->CreateRenderTargetView(resource_.Get(), nullptr, rtvHandle_);
}

void ColorBuffer::Create(const std::wstring& name, uint32_t width, uint32_t height, DXGI_FORMAT format) {
	auto flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	auto desc = DescribeTex2D(width, height, 1, format, flags);

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = format;
	clearValue.Color[0] = clearColor_.GetR();
	clearValue.Color[1] = clearColor_.GetG();
	clearValue.Color[2] = clearColor_.GetB();
	clearValue.Color[3] = clearColor_.GetA();

	CreateTextureResource(name,desc,clearValue);
	CreateViews();
}

void ColorBuffer::CreateViews() {
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	rtvDesc.Format = format_;
	srvDesc.Format = format_;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (arraySize_ > 1) {
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;
		rtvDesc.Texture2DArray.ArraySize = UINT(arraySize_);

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = UINT(arraySize_);
	}
	else {
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}

	auto graphics = GraphicsCore::GetInstance();
	if (rtvHandle_.IsNull()) {
		rtvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	if (srvHandle_.IsNull()) {
		srvHandle_ = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	auto device = graphics->GetDevice();
	device->CreateRenderTargetView(resource_.Get(),&rtvDesc,rtvHandle_);
	device->CreateShaderResourceView(resource_.Get(),&srvDesc,srvHandle_);
}
