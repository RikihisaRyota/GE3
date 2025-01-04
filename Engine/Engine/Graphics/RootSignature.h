#pragma once
/**
 * @file RootSignature.h
 * @brief ルートシグネイチャー用
 */
#include <d3d12.h>
#include <wrl.h>

#include <memory>
#include <string>

class RootSignature {
public:
    // 生成
    void Create(const std::wstring& name, const D3D12_ROOT_SIGNATURE_DESC& desc);
    // Release
    void Release();

    operator ID3D12RootSignature* () const { return rootSignature_.Get(); }
    operator bool() const { return rootSignature_; }

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
};