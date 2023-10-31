#pragma once

#include <d3d12.h>
#include <wrl.h>

class GpuResource {
	friend class CommandContext;
public:
	ID3D12Resource* operator->() { return resource_.Get(); }
	const ID3D12Resource* operator->() const { return resource_.Get(); }

	ID3D12Resource* GetResource() { return resource_.Get(); }
	const ID3D12Resource* GetResource() const { return resource_.Get(); }

	ID3D12Resource** GetAddressOf() { return resource_.GetAddressOf(); }
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	D3D12_RESOURCE_STATES state_;
};