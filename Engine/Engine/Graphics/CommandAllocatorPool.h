#pragma once
/**
 * @file CommandAllocatorPool.h
 * @brief CommandAllocatorのPoolここでCommandAllocatorを管理
 */
#include <d3d12.h>
#include <wrl/client.h>

#include <vector>
#include <queue>
#include <mutex>

class CommandAllocatorPool {
public:
    CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE typ);

    // Allocate
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocate(UINT64 completedFanceValue);
    // Discard
    void Discard(UINT64 fenceValue, const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& commandAllocator);

    size_t GetSize() const { return allocatorPool_.size(); }

private:
    const D3D12_COMMAND_LIST_TYPE type_;
    std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> allocatorPool_;
    std::queue<std::pair<uint64_t, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>> readyAllocators_;
    std::mutex mutex_;
};