#pragma once
#include <string>

#include "GpuResource.h"

class DefaultBuffer :public GpuResource {
public:
	~DefaultBuffer();

	void Create(const std::wstring& name, size_t bufferSize, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
private:
	void Destroy();
};