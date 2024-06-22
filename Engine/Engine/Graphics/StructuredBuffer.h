#pragma once
#include <string>

#include "DescriptorHandle.h"
#include "UploadBuffer.h"

class StructuredBuffer {
public:
	void Create(const std::wstring& name, size_t bufferSize, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	void Copy(const void* srcData, size_t copySize) const;

	UploadBuffer& GetBuffer() { return buffer_; }
	const DescriptorHandle& GetSRV()const { return srvHandle_; }
	void ResetBuffer();
private:
	DescriptorHandle srvHandle_;
	UploadBuffer buffer_;
};