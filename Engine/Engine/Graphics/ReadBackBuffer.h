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

	void Create(const std::wstring& name, size_t bufferSize);
	void Create(const std::wstring& name, size_t numElements, size_t elementSize);


	size_t GetBufferSize()const { return bufferSize_; }
	void* GetCPUData()const { return cpuData_; }

	void ResetBuffer();
private:

	void Destroy();

	size_t bufferSize_;
	void* cpuData_;
};