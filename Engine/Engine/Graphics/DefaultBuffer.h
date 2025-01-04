#pragma once
/**
 * @file DefaultBuffer.h
 * @brief DefaultBuffer
 */
#include <string>

#include "CommandContext.h"
#include "GpuResource.h"
#include "DescriptorHandle.h"


class DefaultBuffer : public GpuResource {
public:
	~DefaultBuffer();
	//	生成
	void Create(const std::wstring& name, size_t bufferSize, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	// View生成
	void CreateUAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);
	void CreateSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	void CreateIndexView(const DXGI_FORMAT& format);
	void CreateVertexView(size_t srideByte);
	void Clear(const QueueType::Type::Param& type, CommandContext& commandContext);
	// BufferSize
	const size_t& GetBufferSize() { return bufferSize_; }
	// ViewGetter
	const DescriptorHandle& GetUAVHandle() { return uavHandle_; }
	const DescriptorHandle& GetSRVHandle() { return srvHandle_; }
	const D3D12_INDEX_BUFFER_VIEW& GetIndexView() { return ibView_; }
	const D3D12_VERTEX_BUFFER_VIEW& GetVertexView() { return vbView_; }
	// CounterOffset
	const UINT& GetCounterOffset() const { return counterOffset_; }
private:
	// パディング
	static inline UINT AlignForUavCounter(UINT bufferSize) {
		const UINT alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
		return (bufferSize + (alignment - 1)) & ~(alignment - 1);
	}
	// 破棄用
	void Destroy();
	DescriptorHandle uavHandle_;
	DescriptorHandle srvHandle_;
	D3D12_INDEX_BUFFER_VIEW  ibView_;
	D3D12_VERTEX_BUFFER_VIEW  vbView_;
	UINT counterOffset_;
	size_t bufferSize_;
};