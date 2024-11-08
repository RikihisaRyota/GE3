#pragma once
#include <string>

#include "GpuResource.h"
#include "DescriptorHandle.h"

class CommandContext;
class DefaultBuffer : public GpuResource {
public:
	~DefaultBuffer();

	void Create(const std::wstring& name, size_t bufferSize, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	void CreateUAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
	void CreateSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	void CreateIndexView(const DXGI_FORMAT& format);
	void CreateVertexView(size_t srideByte);
	void Clear(CommandContext& commandContext);
	const DescriptorHandle& GetUAVHandle() { return uavHandle_; }
	const DescriptorHandle& GetSRVHandle() { return srvHandle_; }
	const D3D12_INDEX_BUFFER_VIEW& GetIndexView() { return ibView_; }
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexView() { return vbView_; }
	const UINT& GetCounterOffset() const { return counterOffset_; }
private:
	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}
	void Destroy();
	DescriptorHandle uavHandle_;
	DescriptorHandle srvHandle_;
	D3D12_INDEX_BUFFER_VIEW  ibView_;
	D3D12_VERTEX_BUFFER_VIEW  vbView_;
	UINT counterOffset_;
	size_t bufferSize_;
};