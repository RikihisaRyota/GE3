#include "CommandSignature.h"
#include <assert.h>
#include "RootSignature.h"
#include "GraphicsCore.h"

void CommandSignature::Create(const std::wstring& name, const D3D12_COMMAND_SIGNATURE_DESC& desc, const RootSignature* rootSignature) {
    ID3D12RootSignature* pRootSignature = rootSignature ? rootSignature->operator ID3D12RootSignature * () : nullptr;
    HRESULT hr = GraphicsCore::GetInstance()->GetDevice()->CreateCommandSignature(
        &desc,
        pRootSignature,
        IID_PPV_ARGS(&commandSignature_));

    assert(SUCCEEDED(hr) && "Failed to create command signature");

    if (SUCCEEDED(hr)) {
        commandSignature_->SetName(name.c_str());
    }
}
