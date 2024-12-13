#pragma once
/**
 * @file DescriptorHandle.h
 * @brief Descriptorを管理するハンドル
 */
#include <d3d12.h>

#define D3D12_CPU_DESCRIPTOR_HANDLE_NULL (D3D12_CPU_DESCRIPTOR_HANDLE(0))
#define D3D12_GPU_DESCRIPTOR_HANDLE_NULL (D3D12_GPU_DESCRIPTOR_HANDLE(0))

class DescriptorHandle {
    friend class DescriptorHeap;
public:
    operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return cpuHandle_; }
    operator D3D12_GPU_DESCRIPTOR_HANDLE() const { return gpuHandle_; }

    bool IsShaderVisible() const { return gpuHandle_.ptr != 0; }
    bool IsNull() const { return cpuHandle_.ptr == 0; }

private:
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_ = D3D12_CPU_DESCRIPTOR_HANDLE_NULL;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_ = D3D12_GPU_DESCRIPTOR_HANDLE_NULL;
};