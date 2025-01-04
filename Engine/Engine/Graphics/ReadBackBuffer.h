#pragma once
/**
 * @file ReadbackBuffer.h
 * @brief ReadBackBuffer用
 */
#include <string>

#include "GpuResource.h"

class ReadBackBuffer :public GpuResource {
public:
	~ReadBackBuffer();
	// 生成
	void Create(const std::wstring& name, size_t bufferSize);
	void Create(const std::wstring& name, size_t numElements, size_t elementSize);

	// Getter
	size_t GetBufferSize()const { return bufferSize_; }
	void* GetCPUData()const { return cpuData_; }
	// ResetBuffer
	void ResetBuffer();
private:
	// Destroy
	void Destroy();

	size_t bufferSize_;
	void* cpuData_;
};