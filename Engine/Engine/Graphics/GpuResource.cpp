#include "GpuResource.h"

#include <assert.h>

#include "GraphicsCore.h"


void GpuResource::CreateResource(const std::wstring& name, const D3D12_HEAP_PROPERTIES& heapProperties, const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE* optimizedClearValue) {
	auto graphics = GraphicsCore::GetInstance();
	auto device = graphics->GetDevice();
	auto hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		initState,
		optimizedClearValue,
		IID_PPV_ARGS(resource_.ReleaseAndGetAddressOf()));
	hr;
	assert(SUCCEEDED(hr) );
	state_ = initState;
	resource_->SetName(name.c_str());
}
