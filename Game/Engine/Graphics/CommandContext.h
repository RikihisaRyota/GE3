#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GPUResource.h"
#include "PipelineState.h"
#include "RootSignature.h"

class CommandContext {
public:
	void Create();

	void Close();
	void Reset();

	void TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState);
	void FlushResourceBarriers();

	void CopyBuffer(GpuResource& dest, GpuResource& src);
	void CopyBufferRegion(GpuResource& dest,UINT64 destOffset, GpuResource& src,UINT64 srcOffset,UINT64 NumBytes = 0) ;

	void ClearColor(ColorBuffer& target);
	void ClearColor(ColorBuffer& target, float Colour[4]);
	void ClearDepth(DepthBuffer& target);
	void ClearDepth(DepthBuffer& target,float clearValue);

	void SetPipelineState(const PipelineState& pipelineState);
	void SetGraphicsRootSignature(const RootSignature& rootSignature);
	void SetComputeRootSignature(const RootSignature& rootSignature);

	void SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]);
	void SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV);
	void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV) { SetRenderTargets(1, &RTV); }
	void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV, D3D12_CPU_DESCRIPTOR_HANDLE DSV) { SetRenderTargets(1, &RTV, DSV); }
	void SetDepthStencilTarget(D3D12_CPU_DESCRIPTOR_HANDLE DSV) { SetRenderTargets(0, nullptr, DSV); }

	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

	void SetGraphicsConstantBuffer(UINT rootIndex,D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetComputeConstantBuffer(UINT rootIndex,D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetGraphicsShaderResource(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetComputeShaderResource(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetGraphicsDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE handle);
	void SetComputeDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE handle);

	void SetComputeUAV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

	void SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbv);
	void SetVertexBuffer(UINT slot, UINT numViews, const D3D12_VERTEX_BUFFER_VIEW vbvs[]);
	void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibv);

	void SetViewport(const D3D12_VIEWPORT& viewport);
	void SetViewport(FLOAT x, FLOAT y, FLOAT w, FLOAT h, FLOAT minDepth = 0.0f, FLOAT maxDepth = 1.0f);
	void SetScissorRect(const D3D12_RECT& rect);
	void SetScissorRect(UINT left, UINT top, UINT right, UINT bottom);
	void SetViewportAndScissorRect(const D3D12_VIEWPORT& viewport, const D3D12_RECT& rect);
	void SetViewportAndScissorRect(UINT x, UINT y, UINT w, UINT h);

	void Draw(UINT vertexCount, UINT vertexStartOffset = 0);
	void DrawIndexed(UINT indexCount, UINT startIndexLocation = 0, INT baseVertexLocation = 0);
	void DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation = 0, UINT startInstanceLocation = 0);
	void DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation = 0, INT baseVertexLocation = 0, UINT startInstanceLocation = 0);
	void ExecuteIndirect(ID3D12CommandSignature* commandSignature, UINT maxCommandCount,ID3D12Resource* argumentBuffer,UINT64 argumentBufferOffset, ID3D12Resource* countBuffer, UINT64 countBufferOffset);

	void Dispatch(uint32_t x, uint32_t y, uint32_t z);

	operator ID3D12GraphicsCommandList* () const { return commandList_.Get(); }
private:
	static const size_t kMaxNumResourceBarriers_ = 16;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

	ID3D12RootSignature* graphicsRootSignature_;
	ID3D12RootSignature* computeRootSignature_;
	ID3D12PipelineState* pipelineState_;

	uint32_t numResourceBarriers_;

	D3D12_RESOURCE_BARRIER resourceBarriers_[kMaxNumResourceBarriers_];
};
