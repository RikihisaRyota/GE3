#include "CommandAllocatorPool.h"

#include <assert.h>

#include "GraphicsCore.h"

CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type) :type_(type) 
{}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandAllocatorPool::Allocate(UINT64 completedFanceValue) {
	std::lock_guard<std::mutex> lock(mutex_);

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;

	if (!readyAllocators_.empty()) {
		const auto& allocatorPair = readyAllocators_.front();
		if (allocatorPair.first <= completedFanceValue) {
			commandAllocator = allocatorPair.second;
			auto hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			readyAllocators_.pop();
		}
	}

	if (!commandAllocator) {
		auto device = GraphicsCore::GetInstance()->GetDevice();
		auto hr = device->CreateCommandAllocator(type_, IID_PPV_ARGS(commandAllocator.GetAddressOf()));
		assert(SUCCEEDED(hr) );

		allocatorPool_.emplace_back(commandAllocator);

		std::wstring allocatorName = L"CommandAllocator" + std::to_wstring(allocatorPool_.size() - 1);
		commandAllocator->SetName(allocatorName.c_str());
	}

	return commandAllocator;
}

void CommandAllocatorPool::Discard(UINT64 fenceValue, const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator) {
	std::lock_guard<std::mutex> LockGuard(mutex_);
	readyAllocators_.push(std::make_pair(fenceValue, commandAllocator));
}
