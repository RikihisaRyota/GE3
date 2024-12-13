#pragma once
/**
 * @file LinerAllocator.h
 * @brief バッファーのメモリの貸し借りを管理
 */
#include "GpuResource.h"

#include <stdint.h>

#include <memory>
#include <vector>
#include <queue>
#include <mutex>

#define DEFAULT_ALIGN 256

struct DynAlloc {
	GpuResource& buffer;
	size_t offset;
	size_t size;
	void* cpuAddress;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
};

struct LinearAllocatorType {
	enum Type {
		kDefault,
		kUpload,
		kNumAllocatorTypes
	}type;
	operator Type() { return type; }
	size_t GetSize() {
		switch (type) {
		case LinearAllocatorType::kDefault:
			return  0x10000;	// 64K
			break;
		case LinearAllocatorType::kUpload:
			return 0x200000;	// 2MB
			break;
		case LinearAllocatorType::kNumAllocatorTypes:
			break;
		default:
			break;
		}
		return (size_t)-1;
	}
	std::wstring GetName() {
		switch (type) {
		case LinearAllocatorType::kDefault:
			return  L"Default";
			break;
		case LinearAllocatorType::kUpload:
			return L"Upload";
			break;
		case LinearAllocatorType::kNumAllocatorTypes:
			break;
		default:
			break;
		}
		return L"";
	}
};


class LinearAllocationPage : public GpuResource {
public:
	void Create(const std::wstring& name, LinearAllocatorType type);

	void* GetCPUAddressStart() const { return cpuAddressStart_; }
	const D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddressStart() const { return gpuAddressStart_; }
	size_t GetSize() const { return size_; }

private:
	void* cpuAddressStart_;
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddressStart_;
	size_t size_;
};

class LinearAllocatorPagePool {
public:
	void Initialize(const LinearAllocatorType& linearAllocatorType);
	void Finalize();
	void Clear();
	void Discard(const D3D12_COMMAND_LIST_TYPE& commandType, UINT64 fenceValue, const std::vector<LinearAllocationPage*>& pages);
	LinearAllocationPage* Allocate(const D3D12_COMMAND_LIST_TYPE& commandType);
private:
	bool CheckAllocate(const D3D12_COMMAND_LIST_TYPE& commandType, LinearAllocationPage** page);
	std::queue<std::pair<uint64_t, LinearAllocationPage*>>& GetPageQueue(D3D12_COMMAND_LIST_TYPE commandType);
	LinearAllocationPage* CreatePage();

	LinearAllocatorType type_;
	std::vector<std::unique_ptr<LinearAllocationPage>> pagePool_;
	std::queue<std::pair<uint64_t, LinearAllocationPage*>> directPageQueue_;
	std::queue<std::pair<uint64_t, LinearAllocationPage*>> computePageQueue_;
	std::queue<std::pair<uint64_t, LinearAllocationPage*>> copyPageQueue_;
	std::mutex mutex_;
};

class LinearAllocator {
public:

	void Create(LinearAllocatorType Type);

	DynAlloc Allocate(const D3D12_COMMAND_LIST_TYPE& commandType, size_t size, size_t alignment = DEFAULT_ALIGN);

	void Reset(D3D12_COMMAND_LIST_TYPE commandType, UINT64 fenceValue);
	
	template <typename T> __forceinline T AlignUpWithMask(T value, size_t mask) {
		return (T)(((size_t)value + mask) & ~mask);
	}
	template <typename T> __forceinline T AlignUp(T value, size_t alignment) {
		return AlignUpWithMask(value, alignment - 1);
	}
private:
	bool HasSpace(size_t size, size_t alignment);


	LinearAllocatorType allocationType_;
	LinearAllocationPage* currentPage_;
	size_t currentOffset_;
	std::vector<LinearAllocationPage*> usedPages_;
};