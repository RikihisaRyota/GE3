#pragma once
/**
 * @file StructuredBuffer.h
 * @brief StructuredBuffer用
 */
#include <string>

#include "DescriptorHandle.h"
#include "UploadBuffer.h"

class StructuredBuffer {
public:
	// 生成
	void Create(const std::wstring& name, size_t bufferSize, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	// コピー
	void Copy(const void* srcData, size_t copySize) const;

	// Getter
	UploadBuffer& GetBuffer() { return buffer_; }
	const DescriptorHandle& GetSRV()const { return srvHandle_; }

	void ResetBuffer();
private:
	DescriptorHandle srvHandle_;
	UploadBuffer buffer_;
};