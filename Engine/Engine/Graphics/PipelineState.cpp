#include "PipelineState.h"

#include <assert.h>

#include "GraphicsCore.h"

void PipelineState::Create(const std::wstring& name, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc) {
	auto hr = GraphicsCore::GetInstance()->GetDevice()->CreateGraphicsPipelineState(
		&desc,
		IID_PPV_ARGS(pipelineState_.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));
	pipelineState_->SetName(name.c_str());
}

void PipelineState::Create(const std::wstring& name, const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc) {
	auto hr = GraphicsCore::GetInstance()->GetDevice()->CreateComputePipelineState(
		&desc,
		IID_PPV_ARGS(pipelineState_.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));
	pipelineState_->SetName(name.c_str());
}

void PipelineState::Release() {
	if (pipelineState_) {
		pipelineState_.Reset();
	}
}
