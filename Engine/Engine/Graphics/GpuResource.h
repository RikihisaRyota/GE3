#pragma once
/**
 * @file GPUResource.h
 * @brief GPUReSourceのための
 */
#include <d3d12.h>
#include <wrl.h>

#include <string>

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

	void CreateResource(
		const std::wstring& name,
		const D3D12_HEAP_PROPERTIES& heapProperties,
		const D3D12_RESOURCE_DESC& desc,
		D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON,
		const D3D12_CLEAR_VALUE* optimizedClearValue = nullptr);
protected:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	D3D12_RESOURCE_STATES state_;
};