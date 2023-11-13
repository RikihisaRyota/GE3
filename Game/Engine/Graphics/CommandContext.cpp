#include "CommandContext.h"

#include <assert.h>

#include "Color.h"
#include "GraphicsCore.h"

void CommandContext::Create() {
	auto device = GraphicsCore::GetInstance()->GetDevice();
	auto hr = device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(commandAllocator_.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));
	hr = device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator_.Get(),
		nullptr,
		IID_PPV_ARGS(commandList_.ReleaseAndGetAddressOf())
	);
	assert(SUCCEEDED(hr));
}

void CommandContext::Close() {
	FlushResourceBarriers();
	auto hr = commandList_->Close();
	assert(SUCCEEDED(hr));
}

void CommandContext::Reset() {
	auto hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));

	auto graphics = GraphicsCore::GetInstance();
	ID3D12DescriptorHeap* ppHeaps[] = {
		static_cast<ID3D12DescriptorHeap*>(graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)),
		static_cast<ID3D12DescriptorHeap*>(graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)),
	};
	commandList_->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	rootSignature_ = nullptr;
	pipelineState_ = nullptr;
}

void CommandContext::TransitionResourse(GpuResource& resource, D3D12_RESOURCE_STATES newState) {
	auto oldState = resource.state_;

	if (newState != oldState) {
		assert(numResourceBarriers_ < kMaxNumResourceBarriers_);
		D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarriers_[numResourceBarriers_++];

		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Transition.pResource = resource.GetResource();
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = oldState;
		barrierDesc.Transition.StateAfter = newState;
		resource.state_ = newState;

	}
	if (numResourceBarriers_ >= kMaxNumResourceBarriers_) {
		FlushResourceBarriers();
	}
}

void CommandContext::FlushResourceBarriers() {
	if (numResourceBarriers_ > 0) {
		commandList_->ResourceBarrier(numResourceBarriers_, resourceBarriers_);
		numResourceBarriers_ = 0;
	}
}

void CommandContext::ClearColor(ColorBuffer& target) {
	FlushResourceBarriers();
	commandList_->ClearRenderTargetView(target.GetRTV(), target.GetClearColor().GetPtr(), 0, nullptr);
}

void CommandContext::ClearColor(ColorBuffer& target, float colour[4]) {
	FlushResourceBarriers();
	commandList_->ClearRenderTargetView(target.GetRTV(), colour, 0, nullptr);
}

void CommandContext::ClearDepth(DepthBuffer& target) {
	FlushResourceBarriers();
	commandList_->ClearDepthStencilView(target.GetDSV(), D3D12_CLEAR_FLAG_DEPTH, target.GetClearValue(), 0, 0, nullptr);
}

void CommandContext::ClearDepth(DepthBuffer& target, float clearValue) {}

void CommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]) {
	commandList_->OMSetRenderTargets(numRTVs, RTVs, FALSE, nullptr);
}

void CommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV) {
	commandList_->OMSetRenderTargets(numRTVs, RTVs, FALSE, &DSV);
}

void CommandContext::SetViewport(const D3D12_VIEWPORT& viewport) {
	commandList_->RSSetViewports(1, &viewport);
}

void CommandContext::SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth, FLOAT maxDepth) {
	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = x;
	viewport.TopLeftY = y;
	viewport.Width = w;
	viewport.Height = h;
	viewport.MinDepth = minDepth;
	viewport.MaxDepth = maxDepth;
	SetViewport(viewport);
}

void CommandContext::SetScissorRect(const D3D12_RECT& rect) {
	assert(rect.left < rect.right && rect.top < rect.bottom);
	commandList_->RSSetScissorRects(1, &rect);
}

void CommandContext::SetScissorRect(UINT left, UINT top, UINT right, UINT bottom) {
	D3D12_RECT rect{};
	rect.left = LONG(left);
	rect.top = LONG(top);
	rect.right = LONG(right);
	rect.bottom = LONG(bottom);
	SetScissorRect(rect);
}

void CommandContext::SetViewportAndScissorRect(const D3D12_VIEWPORT& viewport, const D3D12_RECT& rect) {
	SetViewport(viewport);
	SetScissorRect(rect);
}

void CommandContext::SetViewportAndScissorRect(UINT x, UINT y, UINT w, UINT h) {
	SetViewport(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
	SetScissorRect(x, y, x + w, y + h);
}
