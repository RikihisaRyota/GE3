#pragma once

#include <d3d12.h>
#include <wrl.h>

class GpuResource {
	friend class CommandContext;
public:
	operator ID3D12Resource* () const { return resource_.Get(); }

	ID3D12Resource* operator->() { return resource_.Get(); }
	const ID3D12Resource* operator->() const { return resource_.Get(); }

	ID3D12Resource* GetResource() { return resource_.Get(); }
	const ID3D12Resource* GetResource() const { return resource_.Get(); }

	ID3D12Resource** GetAddressOf() { return resource_.GetAddressOf(); }

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return resource_->GetGPUVirtualAddress(); }

	void SetState(const D3D12_RESOURCE_STATES& state) { state_ = state; }
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	D3D12_RESOURCE_STATES state_;
};