#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
class CommandQueue {
public:
	~CommandQueue();

	void Create();
	void Excute();
	void Signal();
	void WaitForGPU();
private:
	void Shutdown();
private:
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	HANDLE fenceEvent_;
	uint64_t fenceValue_;
};

