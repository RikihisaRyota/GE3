#include "RootSignature.h"

#include <assert.h>

#include "GraphicsCore.h"

void RootSignature::Create(const std::wstring& name, const D3D12_ROOT_SIGNATURE_DESC& desc) {
	Microsoft::WRL::ComPtr<ID3DBlob> blob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	auto result = D3D12SerializeRootSignature(
		&desc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		blob.GetAddressOf(),
		errorBlob.GetAddressOf()
	);
	if (FAILED(result)) {
		// エラーをログに出力するか、デバッグ中に確認する
		if (errorBlob) {
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
			assert(0);
		}
	}

	GraphicsCore::GetInstance()->GetDevice()->CreateRootSignature(
		0,
		blob->GetBufferPointer(),
		blob->GetBufferSize(),
		IID_PPV_ARGS(rootSignature_.ReleaseAndGetAddressOf())
	);
	rootSignature_->SetName(name.c_str());
}

void RootSignature::Release() {
	if (rootSignature_) {
		rootSignature_.Reset();
	}
}
