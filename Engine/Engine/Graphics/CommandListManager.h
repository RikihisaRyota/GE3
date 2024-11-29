#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <chrono>
#include <thread>
#include <array>
#include <queue>
#include <mutex>
#include <stdint.h>
#include <cstdint>

#include "CommandAllocatorPool.h"
#include "CommandContext.h"

#pragma comment(lib,"winmm.lib")

struct FenceDesc;
class CommandQueue {
public:
	friend class CommandListManager;
	friend class CommandContext;
	CommandQueue(D3D12_COMMAND_LIST_TYPE Type);
	~CommandQueue();
	void Create(const std::wstring& name);
	void Shutdown(ID3D12Fence* fence, const HANDLE& handle);

	inline bool IsReady() {
		return commandQueue_ != nullptr;
	}

	uint64_t IncrementFence(ID3D12Fence* fence);
	bool IsFenceComplete(ID3D12Fence* fence, uint64_t FenceValue);
	void StallForFence(ID3D12Fence* fence, uint64_t FenceValue);
	void WaitForFence(ID3D12Fence* fence, const HANDLE& handle, uint64_t FenceValue);
	void WaitForIdle(ID3D12Fence* fence, const HANDLE& handle) { WaitForFence(fence, handle, IncrementFence(fence)); }
	void UpdateFixFPS();
	operator ID3D12CommandQueue* () const { return commandQueue_.Get(); }
	const uint64_t GetLastCompletedFenceValue()const { return lastCompletedFenceValue_; }
private:
	void ExecuteCommandList(ID3D12CommandList* List,const std::string& fenceType);
	void Wait(ID3D12Fence* fence, uint64_t& value, const std::string& fenceType);
	void Signal(ID3D12Fence* fence, uint64_t& value, const std::string& fenceType);

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> RequestAllocator(ID3D12Fence* fence);
	void DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator);

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;

	D3D12_COMMAND_LIST_TYPE type_;

	CommandAllocatorPool allocatorPool_;

	std::mutex fenceMutex_;
	std::mutex eventMutex_;

	uint64_t lastCompletedFenceValue_;
	uint64_t currentSignalValue_;
	std::chrono::steady_clock::time_point referenceTime_;
};

class CommandListManager {
	friend class CommandContext;
public:
	struct FenceDesc {
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
		HANDLE fenceEventHandle;
		uint64_t fenceValue;
	};
public:
	CommandListManager();
	~CommandListManager();

	void Shutdown();

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

	/*void CreateNewCommandList(
		D3D12_COMMAND_LIST_TYPE Type,
		ID3D12GraphicsCommandList** List,
		ID3D12CommandAllocator** Allocator);*/

	bool IsFenceComplete(ID3D12Fence* fence, uint64_t FenceValue) {
		return GetQueue(D3D12_COMMAND_LIST_TYPE(FenceValue >> 56)).IsFenceComplete(fence, FenceValue);
	}

	//void WaitForFence(uint64_t FenceValue);

	void IdleGPU(FenceDesc& desc) {
		graphicsQueue_.WaitForIdle(desc.fence.Get(), desc.fenceEventHandle);
		computeQueue_.WaitForIdle(desc.fence.Get(), desc.fenceEventHandle);
		copyQueue_.WaitForIdle(desc.fence.Get(), desc.fenceEventHandle);
	}

	FenceDesc& GetQueueFenceDesc(const QueueType::Type::Param& type) { return queueFenceDesc_[type]; }
	//std::array<FenceDesc, QueueType::Type::Param::COUNT>& GetAllQueueFence() { return queueFenceDesc_; }
	FenceDesc& GetCopyQueueFence() { return queueFenceDesc_[QueueType::Type::Param::COPY]; }
	FenceDesc& GetComputeQueueFence() { return queueFenceDesc_[QueueType::Type::Param::COMPUTE]; }
	FenceDesc& GetDirectQueueFence() { return queueFenceDesc_[QueueType::Type::Param::DIRECT]; }
	FenceDesc& GetFrameFence() { return frameFenceDesc_; }
private:
	void CreateFence();

	std::array<FenceDesc, QueueType::Type::Param::COUNT> queueFenceDesc_;
	FenceDesc frameFenceDesc_;
	CommandQueue graphicsQueue_;
	CommandQueue computeQueue_;
	CommandQueue copyQueue_;
};