#pragma once

#include <cstdint>
#include <string>

#include "GpuResource.h"

class PixelBuffer :public GpuResource {
public:
    uint32_t GetWidth(void) const { return width_; }
    uint32_t GetHeight(void) const { return height_; }
    uint32_t GetArraySize(void) const { return arraySize_; }
    const DXGI_FORMAT& GetFormat(void) const { return format_; }
protected:
    void CreateTextureResource(const std::wstring& name, const D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE clearValue);
    void AssociateWithResource(const std::wstring& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES state);
    D3D12_RESOURCE_DESC DescribeTex2D(uint32_t width, uint32_t height, uint32_t arraySize, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags);

    uint32_t width_;
    uint32_t height_;
    uint32_t arraySize_;
    DXGI_FORMAT format_;
};