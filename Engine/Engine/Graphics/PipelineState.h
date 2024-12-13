#pragma once
/**
 * @file PipelineState.h
 * @brief Pipeline用のクラス
 */
#include <d3d12.h>
#include <wrl.h>

#include <string>

class PipelineState {
public:
	void Create(const std::wstring& name , const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);

	void Create(const std::wstring& name, const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc);

	void Release();

	operator ID3D12PipelineState* () const { return pipelineState_.Get(); }
	operator bool()const { return pipelineState_.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
};