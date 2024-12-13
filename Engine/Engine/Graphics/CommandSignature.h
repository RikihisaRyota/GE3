#pragma once
/**
 * @file CommandSignature.h
 * @brief CommandSignature
 */
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <string>

class RootSignature;
class CommandSignature {
public:
    void Create(const std::wstring& name, const D3D12_COMMAND_SIGNATURE_DESC& desc, const RootSignature* rootSignature);

    operator ID3D12CommandSignature* () const { return commandSignature_.Get(); }
    operator bool() const { return commandSignature_; }

private:
    Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSignature_;
};
