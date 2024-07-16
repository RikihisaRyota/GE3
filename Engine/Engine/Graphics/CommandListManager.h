#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <chrono>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>
#include <cstdint>

#include "CommandAllocatorPool.h"

#pragma comment(lib,"winmm.lib")

class CommandContext;

class CommandQueue {
public:
	friend class CommandListManager;
	friend class CommandContext;
	CommandQueue(D3D12_COMMAND_LIST_TYPE Type);
	~CommandQueue();
	void Create();
	void Shutdown();

	inline bool IsReady() {
		return commandQueue_ != nullptr;
	}

	uint64_t IncrementFence(void);
	bool IsFenceComplete(uint64_t FenceValue);
	void StallForFence(uint64_t FenceValue);
	void StallForProducer(CommandQueue& Producer);
	void WaitForFence(uint64_t FenceValue);
	void WaitForIdle(void) { WaitForFence(IncrementFence()); }
	void UpdateFixFPS();
	operator ID3D12CommandQueue* () const { return commandQueue_.Get(); }
	const uint64_t GetLastCompletedFenceValue()const { return lastCompletedFenceValue_; }
private:
	uint64_t ExecuteCommandList(ID3D12CommandList* List);
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> RequestAllocator(void);
	void DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator);

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;

	D3D12_COMMAND_LIST_TYPE type_;

	CommandAllocatorPool allocatorPool_;

	std::mutex fenceMutex_;
	std::mutex eventMutex_;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
	uint64_t nextFenceValue_ = 1;
	uint64_t lastCompletedFenceValue_;
	HANDLE fenceEventHandle_;
	std::chrono::steady_clock::time_point referenceTime_;
};

class CommandListManager {
	friend class CommandContext;
public:
	CommandListManager();
	~CommandListManager();

	void Create();

	CommandQueue& GetGraphicsQueue() { return graphicsQueue_; }
	CommandQueue& GetComputeQueue() { return computeQueue_; }
	CommandQueue& GetCopyQueue() { return copyQueue_; }

	CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT) {
		switch (Type) {
		case D3D12_COMMAND_LIST_TYPE_COMPUTE: return computeQueue_;
		case D3D12_COMMAND_LIST_TYPE_COPY: return copyQueue_;
		default: return graphicsQueue_;
		}
	}

	void CreateNewCommandList(
		D3D12_COMMAND_LIST_TYPE Type,
		ID3D12GraphicsCommandList** List,
		ID3D12CommandAllocator** Allocator);

	bool IsFenceComplete(uint64_t FenceValue) {
		return GetQueue(D3D12_COMMAND_LIST_TYPE(FenceValue >> 56)).IsFenceComplete(FenceValue);
	}

	void WaitForFence(uint64_t FenceValue);

	void IdleGPU(void) {
		graphicsQueue_.WaitForIdle();
		computeQueue_.WaitForIdle();
		copyQueue_.WaitForIdle();
	}
private:
	CommandQueue graphicsQueue_;
	CommandQueue computeQueue_;
	CommandQueue copyQueue_;
};