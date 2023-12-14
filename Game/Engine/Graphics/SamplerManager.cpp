#include "SamplerManager.h"

#include "GraphicsCore.h"

namespace SamplerManager {
    DescriptorHandle LinearWrap;
    DescriptorHandle LinearClamp;
    DescriptorHandle LinearBorder;

    DescriptorHandle Anisotropic;

    DescriptorHandle PointWrap;
    DescriptorHandle PointClamp;
    DescriptorHandle PointBorder;

    void Initialize() {
        auto graphics = GraphicsCore::GetInstance();
        auto device = graphics->GetDevice();
        D3D12_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
        samplerDesc.MaxAnisotropy = D3D12_DEFAULT_MAX_ANISOTROPY;
        samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        samplerDesc.BorderColor[0] = 0.0f;
        samplerDesc.BorderColor[1] = 0.0f;
        samplerDesc.BorderColor[2] = 0.0f;
        samplerDesc.BorderColor[3] = 0.0f;
        samplerDesc.MinLOD = 0.0f;
        samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        LinearWrap = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&samplerDesc, LinearWrap);

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        LinearClamp = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&samplerDesc, LinearClamp);

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        LinearBorder = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&samplerDesc, LinearBorder);

        samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        Anisotropic = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&samplerDesc, Anisotropic);

        samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        PointWrap = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&samplerDesc, PointWrap);

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        PointClamp = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&samplerDesc, PointClamp);

        samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        PointBorder = graphics->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        device->CreateSampler(&samplerDesc, PointBorder);
    }
}