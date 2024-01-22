#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <chrono>
#include <thread>

#include <cstdint>

#pragma comment(lib,"winmm.lib")

class CommandContext;

class CommandQueue {
public:
	~CommandQueue();

	void Create();
	void Execute(const CommandContext& commandContext);
	void Signal();
	void WaitForGPU();
	void UpdateFixFPS();

	operator ID3D12CommandQueue* () const { return commandQueue_.Get(); }

private:
	void Shutdown();

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	HANDLE fenceEvent_;
	uint64_t fenceValue_;
	std::chrono::steady_clock::time_point referenceTime_;
};

