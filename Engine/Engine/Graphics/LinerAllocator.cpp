#include "LinerAllocator.h"

#include <assert.h>
#include <thread>
#include <d3dx12.h>

#include "GraphicsCore.h"
#include "CommandListManager.h"


void LinearAllocationPage::Create(const std::wstring& name, LinearAllocatorType type) {
	size_ = type.GetSize();
	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size_);
	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
	if (type == LinearAllocatorType::kDefault) {
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		state = D3D12_RESOURCE_STATE_COMMON;
	}

	CreateResource(name, heapProps, desc, state);

	gpuAddressStart_ = resource_->GetGPUVirtualAddress();
	if (type == LinearAllocatorType::kUpload) {
		resource_->Map(0, nullptr, &cpuAddressStart_);
	}
}

void LinearAllocatorPagePool::Initialize(const LinearAllocatorType& linearAllocatorType) {
	type_ = linearAllocatorType;
}

void LinearAllocatorPagePool::Finalize() {
	Clear();
}

void LinearAllocatorPagePool::Clear() {
	pagePool_.clear();
}

void LinearAllocatorPagePool::Discard(const D3D12_COMMAND_LIST_TYPE& commandType, UINT64 fenceValue, const std::vector<LinearAllocationPage*>& pages) {
	std::lock_guard lock(mutex_);
	auto& readyPageQueue = GetPageQueue(commandType);
	for (auto& page : pages) {
		readyPageQueue.push(std::make_pair(fenceValue, page));
	}
}

LinearAllocationPage* LinearAllocatorPagePool::Allocate(const D3D12_COMMAND_LIST_TYPE& commandType) {
	std::lock_guard lock(mutex_);
	LinearAllocationPage* page = nullptr;
	if (CheckAllocate(commandType, &page)) {
		return page;
	}
	page = CreatePage();
	return page;
}

bool LinearAllocatorPagePool::CheckAllocate(const D3D12_COMMAND_LIST_TYPE& commandType, LinearAllocationPage** page) {

	auto& pageQueue = GetPageQueue(commandType);
	if (!pageQueue.empty()) {
		const auto& [fenceValue, readyPage] = pageQueue.front();
		auto& queue = GraphicsCore::GetInstance()->GetCommandQueue(commandType);

		if (fenceValue <= queue.GetLastCompletedFenceValue()) {
			pageQueue.pop();
			*page = readyPage;
			return true;
		}
	}
	return false;
}

std::queue<std::pair<uint64_t, LinearAllocationPage*>>& LinearAllocatorPagePool::GetPageQueue(D3D12_COMMAND_LIST_TYPE commandType) {
	switch (commandType) {
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return computePageQueue_;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return copyPageQueue_;
	}
	return directPageQueue_;
}

LinearAllocationPage* LinearAllocatorPagePool::CreatePage() {
	auto newPage = pagePool_.emplace_back(std::make_unique<LinearAllocationPage>()).get();
	newPage->Create(L"LinearAllocationPage:" + type_.GetName() + std::to_wstring(pagePool_.size() - 1), type_);

	return newPage;
}

void LinearAllocator::Create(LinearAllocatorType Type) {
	allocationType_ = Type;
}

DynAlloc LinearAllocator::Allocate(const D3D12_COMMAND_LIST_TYPE& commandType, size_t size, size_t alignment) {
	assert(size <= allocationType_.GetSize());

	if (!HasSpace(size, alignment)) {
		currentPage_ = GraphicsCore::GetInstance()->GetLinearAllocatorPagePool(allocationType_).Allocate(commandType);
		currentOffset_ = 0;
		usedPages_.emplace_back(currentPage_);
	}

	size_t alignedSize = AlignUp(size, alignment);
	currentOffset_ = AlignUp(currentOffset_, alignment);

	DynAlloc dynAlloc{
		.buffer = *currentPage_,
		.offset = currentOffset_,
		.size = currentPage_->GetSize(),
		.cpuAddress = static_cast<uint8_t*>(currentPage_->GetCPUAddressStart()) + currentOffset_,
		.gpuAddress = currentPage_->GetGPUAddressStart() + currentOffset_,
	};

	currentOffset_ += alignedSize;

	return dynAlloc;
}

void LinearAllocator::Reset(D3D12_COMMAND_LIST_TYPE commandType, UINT64 fenceValue) {
	currentPage_ = nullptr;
	currentOffset_ = 0;
	if (!usedPages_.empty()) {
		GraphicsCore::GetInstance()->GetLinearAllocatorPagePool(allocationType_).Discard(commandType, fenceValue, usedPages_);
		usedPages_.clear();
	}
}

bool LinearAllocator::HasSpace(size_t size, size_t alignment) {
	if (currentPage_ == nullptr) {
		return false;
	}
	size_t alignedSize = AlignUp(size, alignment);
	size_t alignedOffset = AlignUp(currentOffset_, alignment);

	return alignedOffset + alignedSize <= currentPage_->GetSize();
}
