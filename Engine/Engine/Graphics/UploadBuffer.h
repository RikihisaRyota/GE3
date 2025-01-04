#pragma once
/**
 * @file UploadBuffer.h
 * @brief UploadBuffer用
 */
#include <string>

#include "GpuResource.h"

class UploadBuffer :public GpuResource {
public:
	~UploadBuffer();
	// 生成
	void Create(const std::wstring& name, size_t bufferSize);
	void Create(const std::wstring& name, size_t numElements, size_t elementSize);

	// コピー
	void Copy(const void* srcData, size_t copySize) const;
	template<class T>
	void Copy(const T& srcData)const { Copy(&srcData, sizeof(srcData)); }

	// Getter
	size_t GetBufferSize()const { return bufferSize_; }
	void* GetCPUData()const { return cpuData_; }

	void ResetBuffer();
private:
	// Destroy
	void Destroy();

	size_t bufferSize_;
	void* cpuData_;
};