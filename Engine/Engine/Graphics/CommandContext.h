#pragma once
/**
 * @file CommandContext.h
 * @brief CommandListをラップ
 */
#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

#include <string>

#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GPUResource.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "CommandSignature.h"
#include "LinerAllocator.h"

class ColorBuffer;
class DepthBuffer;
class GPUResource;
class PipelineState;
class RootSignature;
class CommandSignature;
namespace QueueType {
	struct Type {
		enum Param {
			COPY,
			COMPUTE,
			DIRECT,
			COUNT
		};
	};
	inline D3D12_COMMAND_LIST_TYPE GetType(const QueueType::Type::Param& type);
	inline std::string GetTypeString(const QueueType::Type::Param& type);
	inline std::wstring GetTypeWString(const QueueType::Type::Param& type);
}

class CommandContext {
public:
	void Create();

	void StartFrame();
	void BeginDraw();
	void EndFrame();

	void Start();
	void End();
	void Close();
	void Flush();

	void TransitionResource(const QueueType::Type::Param& type,GpuResource& resource, const D3D12_RESOURCE_STATES& newState);
	void UAVBarrier(const QueueType::Type::Param& type, GpuResource& resource);
	void FlushResourceBarriers();

	void ResetBuffer(const QueueType::Type::Param& type, GpuResource& dest, size_t bufferSize);
	void CopyBuffer(const QueueType::Type::Param& type, GpuResource& dest, GpuResource& src);
	void CopyBuffer(const QueueType::Type::Param& type, GpuResource& dest, size_t bufferSize, const void* bufferData);
	void CopyBufferRegion(const QueueType::Type::Param& type, GpuResource& dest, UINT64 destOffset, GpuResource& src, UINT64 srcOffset, UINT64 NumBytes = 0);
	void ReadBackCopyBufferRegion(const QueueType::Type::Param& type, GpuResource& dest, UINT64 destOffset, GpuResource& src, UINT64 srcOffset, UINT64 NumBytes = 0);

	void ClearColor(ColorBuffer& target);
	void ClearColor(ColorBuffer& target, float Colour[4]);
	void ClearDepth(DepthBuffer& target);
	void ClearDepth(DepthBuffer& target, float clearValue);
	void ClearBuffer(const QueueType::Type::Param& type, GpuResource& dest, size_t bufferSize);

	void SetPipelineState(const QueueType::Type::Param& type, const PipelineState& pipelineState);
	void SetGraphicsRootSignature(const RootSignature& rootSignature);
	void SetComputeRootSignature(const QueueType::Type::Param& type,const RootSignature& rootSignature);

	void SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]);
	void SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV);
	void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV) { SetRenderTargets(1, &RTV); }
	void SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE RTV, D3D12_CPU_DESCRIPTOR_HANDLE DSV) { SetRenderTargets(1, &RTV, DSV); }
	void SetDepthStencilTarget(D3D12_CPU_DESCRIPTOR_HANDLE DSV) { SetRenderTargets(0, nullptr, DSV); }

	void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology);

	void SetGraphicsDynamicConstantBufferView(UINT rootIndex, size_t bufferSize, const void* bufferData);
	void SetComputeDynamicConstantBufferView(const QueueType::Type::Param& type, UINT rootIndex, size_t bufferSize, const void* bufferData);
	void SetGraphicsDynamicShaderResource(UINT rootIndex, size_t bufferSize, const void* bufferData);
	void SetComputeDynamicShaderResource(const QueueType::Type::Param& type, UINT rootIndex, size_t bufferSize, const void* bufferData);
	void SetDynamicVertexBuffer(UINT slot, size_t numVertices, size_t vertexStride, const void* vertexData);
	void SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* indexData);
	void SetGraphicsConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetComputeConstantBuffer(const QueueType::Type::Param& type, UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetGraphicsShaderResource(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetComputeShaderResource(const QueueType::Type::Param& type, UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address);
	void SetGraphicsDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE handle);
	void SetComputeDescriptorTable(const QueueType::Type::Param& type,UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE handle);
	void SetDescriptorHeaps(UINT numDescriptorHeaps, ID3D12DescriptorHeap* descriptorHeaps, const QueueType::Type::Param& type);

	void SetComputeUAV(const QueueType::Type::Param& type, uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
	void SetGraphicsUAV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);

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
	void ExecuteIndirect(const CommandSignature& commandSignature, UINT maxCommandCount, ID3D12Resource* argumentBuffer, UINT64 argumentBufferOffset, ID3D12Resource* countBuffer, UINT64 countBufferOffset, const QueueType::Type::Param& type);

	void Dispatch(const QueueType::Type::Param& type,uint32_t x, uint32_t y, uint32_t z);

	void BeginEvent(const QueueType::Type::Param& type, const std::wstring& name);
	void EndEvent(const QueueType::Type::Param& type);

	void SetMarker(const QueueType::Type::Param& type, const std::wstring& name);

	ID3D12GraphicsCommandList* GetCurrentCommandList(const QueueType::Type::Param& type) { return currentCommandList_[type].Get(); }
	ID3D12GraphicsCommandList* GetPreCommandList(const QueueType::Type::Param& type) { return preCommandList_[type].Get(); }
private:

	static const size_t kMaxNumResourceBarriers_ = 16;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> currentCommandAllocator_[QueueType::Type::COUNT];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> preCommandAllocator_[QueueType::Type::COUNT];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> currentCommandList_[QueueType::Type::COUNT];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> preCommandList_[QueueType::Type::COUNT];

	ID3D12RootSignature* graphicsRootSignature_;
	ID3D12RootSignature* computeRootSignature_;
	ID3D12PipelineState* directPipelineState_;
	ID3D12PipelineState* computePipelineState_;

	struct ResourceBarrier {
		uint32_t numResourceBarriers;
		D3D12_RESOURCE_BARRIER resourceBarrier[kMaxNumResourceBarriers_];
	};

	ResourceBarrier resourceBarriers_[QueueType::Type::COUNT];

	LinearAllocator currentDynamicBuffers_[QueueType::Type::COUNT][LinearAllocatorType::kNumAllocatorTypes];
	LinearAllocator previousDynamicBuffers_[QueueType::Type::COUNT][LinearAllocatorType::kNumAllocatorTypes];

	bool isFirstFrameComplete_;
};