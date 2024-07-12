#pragma once

#include "GpuResource.h"

#include <stdint.h>

#include <memory>
#include <vector>
#include <queue>

class LinearAllocationPage : public GpuResource {
public:
	LinearAllocationPage(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Usage) : GpuResource() {
		resource_.Attach(pResource);
		state_ = Usage;
		resource_->Map(0, nullptr, &cpuVirtualAddress);
	}

	~LinearAllocationPage() {
		Unmap();
	}

	void Map(void) {
		if (cpuVirtualAddress == nullptr) {
			resource_->Map(0, nullptr, &cpuVirtualAddress);
		}
	}

	void Unmap(void) {
		if (cpuVirtualAddress != nullptr) {
			resource_->Unmap(0, nullptr);
			cpuVirtualAddress = nullptr;
		}
	}

	void* cpuVirtualAddress;
};

class LinearAllocatorPageManager {
public:
	LinearAllocatorPageManager();
	LinearAllocationPage* RequestPage(void);
	LinearAllocationPage* CreateNewPage(size_t PageSize = 0);

	void DiscardPages(uint64_t FenceID, const std::vector<LinearAllocationPage*>& Pages);

	void FreeLargePages(uint64_t FenceID, const std::vector<LinearAllocationPage*>& Pages);

	void Destroy(void) { pagePool_.clear(); }
private:
	std::vector<std::unique_ptr<LinearAllocationPage>> pagePool_;
	std::queue<std::pair<uint64_t, LinearAllocationPage*> > retirePages_;
	std::queue<std::pair<uint64_t, LinearAllocationPage*> > deletionQueue_;
};