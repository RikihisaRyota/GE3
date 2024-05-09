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
	computeRootSignature_ = nullptr;
	graphicsRootSignature_ = nullptr;
	pipelineState_ = nullptr;
}

void CommandContext::TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState) {
	auto oldState = resource.state_;

	if (newState != oldState) {
		assert(numResourceBarriers_ < kMaxNumResourceBarriers_);
		D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarriers_[numResourceBarriers_++];
		barrierDesc = D3D12_RESOURCE_BARRIER{};
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
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

void CommandContext::UAVBarrier(GpuResource& resource) {
	D3D12_RESOURCE_BARRIER& barrierDesc = resourceBarriers_[numResourceBarriers_++];
	barrierDesc = D3D12_RESOURCE_BARRIER{};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.UAV.pResource = resource.GetResource();
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

void CommandContext::CopyBuffer(GpuResource& dest, GpuResource& src) {
	TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	commandList_->CopyResource(dest, src);
}

void CommandContext::CopyBufferRegion(GpuResource& dest, UINT64 destOffset, GpuResource& src, UINT64 srcOffset, UINT64 NumBytes) {
	TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	commandList_->CopyBufferRegion(dest, destOffset, src, srcOffset, NumBytes);
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

void CommandContext::SetPipelineState(const PipelineState& pipelineState) {
	ID3D12PipelineState* ps = pipelineState;
	if (pipelineState_ != ps) {
		pipelineState_ = ps;
		commandList_->SetPipelineState(pipelineState_);
	}
}

void CommandContext::SetGraphicsRootSignature(const RootSignature& rootSignature) {
	ID3D12RootSignature* rs = rootSignature;
	if (graphicsRootSignature_ != rs) {
		graphicsRootSignature_ = rs;
		commandList_->SetGraphicsRootSignature(graphicsRootSignature_);
	}
}

void CommandContext::SetComputeRootSignature(const RootSignature& rootSignature) {
	ID3D12RootSignature* rs = rootSignature;
	if (computeRootSignature_ != rs) {
		computeRootSignature_ = rs;
		commandList_->SetComputeRootSignature(computeRootSignature_);
	}
}

void CommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]) {
	commandList_->OMSetRenderTargets(numRTVs, RTVs, FALSE, nullptr);
}

void CommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV) {
	commandList_->OMSetRenderTargets(numRTVs, RTVs, FALSE, &DSV);
}

void CommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) {
	commandList_->IASetPrimitiveTopology(topology);
}

void CommandContext::SetGraphicsConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
	commandList_->SetGraphicsRootConstantBufferView(rootIndex, address);
}

void CommandContext::SetComputeConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
	commandList_->SetComputeRootConstantBufferView(rootIndex, address);
}

void CommandContext::SetGraphicsShaderResource(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
	commandList_->SetGraphicsRootShaderResourceView(rootIndex, address);
}

void CommandContext::SetComputeShaderResource(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
	commandList_->SetComputeRootShaderResourceView(rootIndex, address);
}

void CommandContext::SetGraphicsDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE address) {
	commandList_->SetGraphicsRootDescriptorTable(rootIndex, address);
}

void CommandContext::SetComputeDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE address) {
	commandList_->SetComputeRootDescriptorTable(rootIndex, address);
}

void CommandContext::SetDescriptorHeaps(UINT numDescriptorHeaps, ID3D12DescriptorHeap* descriptorHeaps) {
	commandList_->SetDescriptorHeaps(numDescriptorHeaps, &descriptorHeaps);
}

void CommandContext::SetComputeUAV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation) {
	commandList_->SetComputeRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void CommandContext::SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbv) {
	commandList_->IASetVertexBuffers(slot, 1, &vbv);
}

void CommandContext::SetVertexBuffer(UINT slot, UINT numViews, const D3D12_VERTEX_BUFFER_VIEW vbvs[]) {
	commandList_->IASetVertexBuffers(slot, numViews, vbvs);
}

void CommandContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibv) {
	commandList_->IASetIndexBuffer(&ibv);
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

void CommandContext::Draw(UINT vertexCount, UINT vertexStartOffset) {
	DrawInstanced(vertexCount, 1, vertexStartOffset, 0);
}

void CommandContext::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation) {
	DrawIndexedInstanced(indexCount, 1, startIndexLocation, baseVertexLocation, 0);
}

void CommandContext::DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation) {
	FlushResourceBarriers();
	commandList_->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void CommandContext::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation) {
	FlushResourceBarriers();
	commandList_->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandContext::ExecuteIndirect(ID3D12CommandSignature* commandSignature, UINT maxCommandCount, ID3D12Resource* argumentBuffer, UINT64 argumentBufferOffset, ID3D12Resource* countBuffer, UINT64 countBufferOffset) {
	FlushResourceBarriers();
	commandList_->ExecuteIndirect(commandSignature, maxCommandCount, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
}

void CommandContext::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
	FlushResourceBarriers();
	commandList_->Dispatch(x, y, z);
}
