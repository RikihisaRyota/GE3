#include "CommandContext.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <cassert>

//#include <pix3.h>

#include "Color.h"
#include "GraphicsCore.h"
#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GPUResource.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "CommandSignature.h"
#include "CommandListManager.h"


void CommandContext::Create() {
	auto device = GraphicsCore::GetInstance()->GetDevice();
	for (uint32_t type = 0; type < QueueType::Type::COUNT; type++) {
		auto hr = device->CreateCommandAllocator(
			GetType(QueueType::Type::Param(type)),
			IID_PPV_ARGS(currentCommandAllocator_[type].ReleaseAndGetAddressOf())
		);
		hr = device->CreateCommandList(
			0,
			GetType(QueueType::Type::Param(type)),
			currentCommandAllocator_[type].Get(),
			nullptr,
			IID_PPV_ARGS(currentCommandList_[type].ReleaseAndGetAddressOf())
		);
		assert(SUCCEEDED(hr));
		std::wstring s = L"currentCommandAllocator" + QueueType::GetTypeWString(QueueType::Type::Param(type));
		currentCommandAllocator_[type]->SetName(s.c_str());
		s = L"currentCommandList" + QueueType::GetTypeWString(QueueType::Type::Param(type));
		currentCommandList_[type]->SetName(s.c_str());

		currentCommandList_[type]->Close();
		//currentCommandList_[type]->Reset(currentCommandAllocator_[type].Get(), nullptr);

		hr = device->CreateCommandAllocator(
			GetType(QueueType::Type::Param(type)),
			IID_PPV_ARGS(preCommandAllocator_[type].ReleaseAndGetAddressOf())
		);
		assert(SUCCEEDED(hr));
		hr = device->CreateCommandList(
			0,
			GetType(QueueType::Type::Param(type)),
			preCommandAllocator_[type].Get(),
			nullptr,
			IID_PPV_ARGS(preCommandList_[type].ReleaseAndGetAddressOf())
		);
		assert(SUCCEEDED(hr));
		s = L"preCommandAllocator" + QueueType::GetTypeWString(QueueType::Type::Param(type));
		preCommandAllocator_[type]->SetName(s.c_str());
		s = L"preCommandList" + QueueType::GetTypeWString(QueueType::Type::Param(type));
		preCommandList_[type]->SetName(s.c_str());

		preCommandList_[type]->Close();
		//preCommandList_[type]->Reset(preCommandAllocator_[type].Get(), nullptr);
		// ダイナミックバッファを作成
		for (uint32_t i = 0; i < LinearAllocatorType::kNumAllocatorTypes; ++i) {
			currentDynamicBuffers_[type][i].Create(LinearAllocatorType(static_cast<LinearAllocatorType::Type>(i)));
			previousDynamicBuffers_[type][i].Create(LinearAllocatorType(static_cast<LinearAllocatorType::Type>(i)));
		}

	}
}

void CommandContext::StartFrame() {
	auto graphics = GraphicsCore::GetInstance();

	// Queueを取得
	for (int32_t t = 0; t < QueueType::Type::COUNT; t++) {
		// QueueType
		auto queueType = QueueType::Type::Param(t);
		ExecuteCommandList(queueType);
	}

	// 新しいフレームの準備
	for (int32_t t = 0; t < QueueType::Type::COUNT; t++) {
		auto queueType = QueueType::Type::Param(t);
		ResetCommandList(queueType);
	}
	computeRootSignature_ = nullptr;
	graphicsRootSignature_ = nullptr;
	directPipelineState_ = nullptr;
	computePipelineState_ = nullptr;
}

void CommandContext::BeginDraw() {
	auto graphics = GraphicsCore::GetInstance();

	// Queueを取得
	auto& copyQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::COPY));
	auto& computeQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::COMPUTE));
	auto& directQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::DIRECT));

	// CopyQueue
	auto queueType = QueueType::Type::Param::COPY;
	auto& copyQueueFence = graphics->GetCommandListManager().GetCopyQueueFence();

	// Signal発行
	copyQueue.Signal(copyQueueFence.fence.Get(), copyQueueFence.fenceValue, QueueType::GetTypeString(queueType));
	// DirectにWaitを発行
	directQueue.Wait(copyQueueFence.fence.Get(), copyQueueFence.fenceValue, QueueType::GetTypeString(QueueType::Type::Param::DIRECT));

	// ComputeQueue
	queueType = QueueType::Type::Param::COMPUTE;
	auto& computeQueueFence = graphics->GetCommandListManager().GetComputeQueueFence();

	// Signal発行
	computeQueue.Signal(computeQueueFence.fence.Get(), computeQueueFence.fenceValue, QueueType::GetTypeString(queueType));
	// DirectにWaitを発行
	directQueue.Wait(computeQueueFence.fence.Get(), computeQueueFence.fenceValue, QueueType::GetTypeString(QueueType::Type::Param::DIRECT));

	// DirectQueue
	queueType = QueueType::Type::Param::DIRECT;
	auto& directQueueFence = graphics->GetCommandListManager().GetDirectQueueFence();

	// Signal発行
	directQueue.Signal(directQueueFence.fence.Get(), directQueueFence.fenceValue, QueueType::GetTypeString(QueueType::Type::Param::DIRECT));
	// Waitを発行Update処理が終わってることを確認
	directQueue.Wait(directQueueFence.fence.Get(), directQueueFence.fenceValue, QueueType::GetTypeString(QueueType::Type::Param::DIRECT));
}

void CommandContext::EndFrame() {
	auto graphics = GraphicsCore::GetInstance();
	auto& frameFence = graphics->GetCommandListManager().GetFrameFence();
	auto& copyQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::COPY));
	auto& computeQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::COMPUTE));
	auto& directQueue = graphics->GetCommandQueue(GetType(QueueType::Type::Param::DIRECT));

	directQueue.Signal(frameFence.fence.Get(), frameFence.fenceValue, QueueType::GetTypeString(QueueType::Type::Param::DIRECT));

	int64_t preFenceValue = int64_t(frameFence.fenceValue);

	directQueue.WaitForFence(frameFence.fence.Get(), frameFence.fenceEventHandle, preFenceValue);
	// 既に終了しているvalueをそれぞれに格納
	computeQueue.SetFenceComplete(preFenceValue);
	copyQueue.SetFenceComplete(preFenceValue);

	for (uint32_t t = 0; t < QueueType::Type::COUNT; t++) {
		auto commandListType = GetType(QueueType::Type::Param(t));
		auto& queue = graphics->GetCommandQueue(commandListType);
		// 前回のフレームのリソースをリセット
		queue.allocatorPool_.Discard(preFenceValue, preCommandAllocator_[t]);
		for (uint32_t i = 0; i < LinearAllocatorType::kNumAllocatorTypes; ++i) {
			previousDynamicBuffers_[t][i].Reset(QueueType::GetType(QueueType::Type::Param(t)), preFenceValue);
		}
	}
}

void CommandContext::Start() {
	auto graphics = GraphicsCore::GetInstance();
	for (uint32_t type = 0; type < QueueType::Type::COUNT; type++) {
		// 新しいコマンドアロケータを取得し、コマンドリストをリセット
		//currentCommandAllocator_[type] = queue.allocatorPool_.Allocate(queue.GetLastCompletedFenceValue(FenceType::Type::Frame));
		currentCommandList_[type]->Reset(currentCommandAllocator_[type].Get(), nullptr);

		// 現在のダイナミックバッファを作成
		for (uint32_t i = 0; i < LinearAllocatorType::kNumAllocatorTypes; ++i) {
			currentDynamicBuffers_[type][i].Create(LinearAllocatorType(static_cast<LinearAllocatorType::Type>(i)));
		}

		if (type != QueueType::Type::COPY) {
			// ディスクリプタヒープを設定
			ID3D12DescriptorHeap* ppHeaps[] = {
				static_cast<ID3D12DescriptorHeap*>(graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)),
				static_cast<ID3D12DescriptorHeap*>(graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)),
			};
			currentCommandList_[type]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		}
	}
	computeRootSignature_ = nullptr;
	graphicsRootSignature_ = nullptr;
	directPipelineState_ = nullptr;
	computePipelineState_ = nullptr;
}


void CommandContext::End() {
	//// 使ってない整備してない
	//auto graphics = GraphicsCore::GetInstance();
	//auto& directFenceDesc = graphics->GetCommandListManager().GetQueueFenceDesc(QueueType::Type::DIRECT);
	//int64_t preFenceValue = int64_t(directFenceDesc.fenceValue) - 1;
	//if (preFenceValue >= 0) {
	//	for (uint32_t t = 0; t < QueueType::Type::COUNT; t++) {
	//		auto commandListType = GetType(QueueType::Type::Param(t));
	//		auto queueType = QueueType::Type::Param(t);
	//		auto& queue = graphics->GetCommandQueue(commandListType);

	//		// 前回のフレームの処理が終わっているかチェックする
	//		queue.WaitForFence(directFenceDesc.fence.Get(), directFenceDesc.fenceEventHandle, preFenceValue);

	//		// フレームの終わりにスワップ
	//		std::swap(currentCommandAllocator_[t], preCommandAllocator_[t]);
	//		std::swap(currentCommandList_[t], preCommandList_[t]);

	//		// 前回のフレームのリソースをリセット
	//		for (uint32_t i = 0; i < LinearAllocatorType::kNumAllocatorTypes; ++i) {
	//			previousDynamicBuffers_[t][i].Reset(commandListType, preFenceValue);
	//			// 現在のバッファと前回のバッファを入れ替える
	//			std::swap(currentDynamicBuffers_[t][i], previousDynamicBuffers_[t][i]);
	//		}
	//		preFenceValue++;
	//	}
	//}
}

void CommandContext::Close() {
	FlushResourceBarriers();
	for (uint32_t i = 0; i < QueueType::Type::COUNT; i++) {
		auto hr = currentCommandList_[i]->Close();
		if (FAILED(hr)) {
			//// システムメモリの使用状況をログに記録
			//MEMORYSTATUSEX statex;
			//statex.dwLength = sizeof(MEMORYSTATUSEX);
			//GlobalMemoryStatusEx(&statex);

			//std::ofstream logFile("log.txt", std::ios::app);
			//logFile << "Close failed for command list " << i << ", HRESULT: " << hr << std::endl;
			//logFile << "システムメモリ使用率: " << statex.dwMemoryLoad << "%" << std::endl;
			//logFile << "使用可能な物理メモリ: " << statex.ullAvailPhys / (1024 * 1024) << " MB" << std::endl;
			//logFile << "総システムメモリ: " << statex.ullTotalPhys / (1024 * 1024) << " MB" << std::endl;
			//logFile.close();


			//// エラーメッセージを取得して出力
			//TCHAR errorMsg[256];
			//FormatMessage(
			//	FORMAT_MESSAGE_FROM_SYSTEM,
			//	NULL,
			//	hr,
			//	0,
			//	errorMsg,
			//	sizeof(errorMsg) / sizeof(TCHAR),
			//	NULL
			//);
			//OutputDebugString(errorMsg);
		}
		assert(SUCCEEDED(hr));
	}
}

void CommandContext::Close(const QueueType::Type::Param& type)
{
	auto hr = currentCommandList_[type]->Close();
	assert(SUCCEEDED(hr));
}


void CommandContext::Flush() {
	auto graphics = GraphicsCore::GetInstance();
	auto& frameFenceDesc = graphics->GetCommandListManager().GetFrameFence();
	for (uint32_t t = 0; t < QueueType::Type::COUNT; t++) {
		auto commandListType = GetType(QueueType::Type::Param(t));
		auto queueType = QueueType::Type::Param(t);
		auto& queue = graphics->GetCommandQueue(commandListType);

		queue.ExecuteCommandList(currentCommandList_[t].Get(), QueueType::GetTypeString(queueType));
		queue.Signal(frameFenceDesc.fence.Get(), frameFenceDesc.fenceValue, QueueType::GetTypeString(queueType));
		//queue.Wait(frameFenceDesc.fence.Get(), frameFenceDesc.fenceValue, QueueType::GetTypeString(queueType));
		queue.WaitForFence(frameFenceDesc.fence.Get(), frameFenceDesc.fenceEventHandle, frameFenceDesc.fenceValue);
		// フレームの終わりにスワップ
		std::swap(currentCommandAllocator_[t], preCommandAllocator_[t]);
		std::swap(currentCommandList_[t], preCommandList_[t]);
		// 前回のフレームのリソースをリセット
		for (uint32_t i = 0; i < LinearAllocatorType::kNumAllocatorTypes; ++i) {
			previousDynamicBuffers_[t][i].Reset(commandListType, frameFenceDesc.fenceValue);
			// 現在のバッファと前回のバッファを入れ替える
			std::swap(currentDynamicBuffers_[t][i], previousDynamicBuffers_[t][i]);
		}
	}
}

void CommandContext::ExecuteCommandList(const QueueType::Type::Param& type)
{
	auto graphics = GraphicsCore::GetInstance();
	// Queue
	auto& queue = graphics->GetCommandQueue(QueueType::GetType(type));
	// 実行
	queue.ExecuteCommandList(currentCommandList_[type].Get(), QueueType::GetTypeString(type));
}

void CommandContext::ResetCommandList(const QueueType::Type::Param& type)
{
	auto graphics = GraphicsCore::GetInstance();
	auto commandListType = GetType(type);
	auto& queue = graphics->GetCommandQueue(commandListType);
	// pre代入
	std::swap(currentCommandAllocator_[type], preCommandAllocator_[type]);
	std::swap(currentCommandList_[type], preCommandList_[type]);
	for (uint32_t i = 0; i < LinearAllocatorType::kNumAllocatorTypes; ++i) {
		std::swap(currentDynamicBuffers_[type][i], previousDynamicBuffers_[type][i]);
	}

	currentCommandAllocator_[type] = queue.allocatorPool_.Allocate(queue.GetLastCompletedFenceValue());
	// コマンドリストをリセット
	currentCommandList_[type]->Reset(currentCommandAllocator_[type].Get(), nullptr);

	if (type != QueueType::Type::COPY) {
		// ディスクリプタヒープを設定
		ID3D12DescriptorHeap* ppHeaps[] = {
			static_cast<ID3D12DescriptorHeap*>(graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)),
			static_cast<ID3D12DescriptorHeap*>(graphics->GetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER)),
		};
		currentCommandList_[type]->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	}
}

void CommandContext::TransitionResource(const QueueType::Type::Param& type, GpuResource& resource, const D3D12_RESOURCE_STATES& newState) {
	auto oldState = resource.state_;
	auto& barrier = resourceBarriers_[type];
	if (newState != oldState) {
		assert(barrier.numResourceBarriers < kMaxNumResourceBarriers_);
		D3D12_RESOURCE_BARRIER& barrierDesc = barrier.resourceBarrier[barrier.numResourceBarriers++];
		barrierDesc = D3D12_RESOURCE_BARRIER{};
		barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrierDesc.Transition.pResource = resource.GetResource();
		barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrierDesc.Transition.StateBefore = oldState;
		barrierDesc.Transition.StateAfter = newState;
		resource.state_ = newState;

	}
	if (barrier.numResourceBarriers >= kMaxNumResourceBarriers_) {
		FlushResourceBarriers();
	}
}

void CommandContext::UAVBarrier(const QueueType::Type::Param& type, GpuResource& resource) {
	auto& barrier = resourceBarriers_[type];
	D3D12_RESOURCE_BARRIER& barrierDesc = barrier.resourceBarrier[barrier.numResourceBarriers++];
	barrierDesc = D3D12_RESOURCE_BARRIER{};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.UAV.pResource = resource.GetResource();
	if (resourceBarriers_->numResourceBarriers >= kMaxNumResourceBarriers_) {
		FlushResourceBarriers();
	}
}

void CommandContext::FlushResourceBarriers() {
	for (uint32_t i = 0; auto & barrier : resourceBarriers_) {
		if (barrier.numResourceBarriers > 0) {
			currentCommandList_[i]->ResourceBarrier(barrier.numResourceBarriers, barrier.resourceBarrier);
			barrier.numResourceBarriers = 0;
		}
		i++;
	}
}

void CommandContext::ResetBuffer(const QueueType::Type::Param& type, GpuResource& dest, size_t bufferSize) {
	auto allocation = currentDynamicBuffers_[type][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(type), bufferSize, 256);
	memset(allocation.cpuAddress, 0, bufferSize);
	CopyBufferRegion(type, dest, 0, allocation.buffer, allocation.offset, bufferSize);
}

void CommandContext::CopyBuffer(const QueueType::Type::Param& type, GpuResource& dest, GpuResource& src) {
	TransitionResource(type, dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(type, src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	currentCommandList_[type]->CopyResource(dest, src);
	UAVBarrier(type, dest);
}

void CommandContext::CopyBuffer(const QueueType::Type::Param& type, GpuResource& dest, size_t bufferSize, const void* bufferData) {
	assert(bufferData);
	auto allocation = currentDynamicBuffers_[type][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(type), bufferSize, 256);
	memcpy(allocation.cpuAddress, bufferData, bufferSize);
	CopyBufferRegion(type, dest, 0, allocation.buffer, allocation.offset, bufferSize);
}

void CommandContext::CopyBufferRegion(const QueueType::Type::Param& type, GpuResource& dest, UINT64 destOffset, GpuResource& src, UINT64 srcOffset, UINT64 NumBytes) {
	TransitionResource(type, dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(type, src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	currentCommandList_[type]->CopyBufferRegion(dest, destOffset, src, srcOffset, NumBytes);
	UAVBarrier(type, dest);
}

void CommandContext::CopyBufferRegion(const QueueType::Type::Param& type, GpuResource& dest, UINT64 destOffset, size_t bufferSize, const void* bufferData, UINT64 NumBytes)
{
	assert(bufferData);
	auto allocation = currentDynamicBuffers_[type][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(type), bufferSize, 256);
	memcpy(allocation.cpuAddress, bufferData, bufferSize);
	CopyBufferRegion(type, dest, destOffset, allocation.buffer, allocation.offset, bufferSize);
}

void CommandContext::ReadBackCopyBufferRegion(const QueueType::Type::Param& type, GpuResource& dest, UINT64 destOffset, GpuResource& src, UINT64 srcOffset, UINT64 NumBytes) {
	TransitionResource(type, dest, D3D12_RESOURCE_STATE_COPY_DEST);
	TransitionResource(type, src, D3D12_RESOURCE_STATE_COPY_SOURCE);
	FlushResourceBarriers();
	currentCommandList_[type]->CopyBufferRegion(dest, destOffset, src, srcOffset, NumBytes);
}

void CommandContext::ClearColor(ColorBuffer& target) {
	FlushResourceBarriers();
	currentCommandList_[QueueType::Type::DIRECT]->ClearRenderTargetView(target.GetRTV(), target.GetClearColor().GetPtr(), 0, nullptr);
}

void CommandContext::ClearColor(ColorBuffer& target, float colour[4]) {
	FlushResourceBarriers();
	currentCommandList_[QueueType::Type::DIRECT]->ClearRenderTargetView(target.GetRTV(), colour, 0, nullptr);
}

void CommandContext::ClearDepth(DepthBuffer& target) {
	FlushResourceBarriers();
	currentCommandList_[QueueType::Type::DIRECT]->ClearDepthStencilView(target.GetDSV(), D3D12_CLEAR_FLAG_DEPTH, target.GetClearValue(), 0, 0, nullptr);
}

void CommandContext::ClearDepth(DepthBuffer& target, float clearValue) {
	target;
	clearValue;
}

void CommandContext::ClearBuffer(const QueueType::Type::Param& type, GpuResource& dest, size_t bufferSize) {
	auto allocation = currentDynamicBuffers_[type][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(type), bufferSize, 256);
	memset(allocation.cpuAddress, 0, bufferSize);
	CopyBufferRegion(type, dest, 0, allocation.buffer, allocation.offset, bufferSize);
}

void CommandContext::SetPipelineState(const QueueType::Type::Param& type, const PipelineState& pipelineState) {
	ID3D12PipelineState* ps = pipelineState;
	switch (type)
	{
	case QueueType::Type::DIRECT:
		if (directPipelineState_ != ps) {
			directPipelineState_ = ps;
			currentCommandList_[type]->SetPipelineState(directPipelineState_);
		}
		break;
	case QueueType::Type::COMPUTE:
	case QueueType::Type::SPAWN:
		if (computePipelineState_ != ps) {
			computePipelineState_ = ps;
			currentCommandList_[type]->SetPipelineState(computePipelineState_);
		}
		break;
	default:
		break;
	}
}

void CommandContext::SetGraphicsRootSignature(const RootSignature& rootSignature) {
	ID3D12RootSignature* rs = rootSignature;
	if (graphicsRootSignature_ != rs) {
		graphicsRootSignature_ = rs;
		currentCommandList_[QueueType::Type::DIRECT]->SetGraphicsRootSignature(graphicsRootSignature_);
	}
}

void CommandContext::SetComputeRootSignature(const QueueType::Type::Param& type, const RootSignature& rootSignature) {
	ID3D12RootSignature* rs = rootSignature;
	switch (type)
	{
		// 変えてね
	case QueueType::Type::DIRECT:
		if (graphicsRootSignature_ != rs) {
			graphicsRootSignature_ = rs;
			currentCommandList_[type]->SetComputeRootSignature(graphicsRootSignature_);
		}
		break;
	case QueueType::Type::COMPUTE:
	case QueueType::Type::SPAWN:
		if (computeRootSignature_ != rs) {
			computeRootSignature_ = rs;
			currentCommandList_[type]->SetComputeRootSignature(computeRootSignature_);
		}
		break;
	default:
		break;
	}
}

void CommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[]) {
	currentCommandList_[QueueType::Type::DIRECT]->OMSetRenderTargets(numRTVs, RTVs, FALSE, nullptr);
}

void CommandContext::SetRenderTargets(UINT numRTVs, const D3D12_CPU_DESCRIPTOR_HANDLE RTVs[], D3D12_CPU_DESCRIPTOR_HANDLE DSV) {
	currentCommandList_[QueueType::Type::DIRECT]->OMSetRenderTargets(numRTVs, RTVs, FALSE, &DSV);
}

void CommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) {
	currentCommandList_[QueueType::Type::DIRECT]->IASetPrimitiveTopology(topology);
}

void CommandContext::SetGraphicsDynamicConstantBufferView(UINT rootIndex, size_t bufferSize, const void* bufferData) {
	assert(bufferData);

	auto allocation = currentDynamicBuffers_[QueueType::Type::DIRECT][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(QueueType::Type::DIRECT), bufferSize, 256);
	memcpy(allocation.cpuAddress, bufferData, bufferSize);
	TransitionResource(QueueType::Type::DIRECT, allocation.buffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	currentCommandList_[QueueType::Type::DIRECT]->SetGraphicsRootConstantBufferView(rootIndex, allocation.gpuAddress);
}

void CommandContext::SetComputeDynamicConstantBufferView(const QueueType::Type::Param& type, UINT rootIndex, size_t bufferSize, const void* bufferData) {
	assert(bufferData);

	auto allocation = currentDynamicBuffers_[type][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(QueueType::Type::COMPUTE), bufferSize, 256);
	memcpy(allocation.cpuAddress, bufferData, bufferSize);
	TransitionResource(type, allocation.buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	currentCommandList_[type]->SetComputeRootConstantBufferView(rootIndex, allocation.gpuAddress);
}

void CommandContext::SetGraphicsDynamicShaderResource(UINT rootIndex, size_t bufferSize, const void* bufferData) {
	assert(bufferData);

	auto allocation = currentDynamicBuffers_[QueueType::Type::DIRECT][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(QueueType::Type::DIRECT), bufferSize, 256);
	memcpy(allocation.cpuAddress, bufferData, bufferSize);
	TransitionResource(QueueType::Type::DIRECT, allocation.buffer, D3D12_RESOURCE_STATE_GENERIC_READ);
	currentCommandList_[QueueType::Type::DIRECT]->SetGraphicsRootShaderResourceView(rootIndex, allocation.gpuAddress);
}

void CommandContext::SetComputeDynamicShaderResource(const QueueType::Type::Param& type, UINT rootIndex, size_t bufferSize, const void* bufferData) {
	assert(bufferData);
	auto allocation = currentDynamicBuffers_[type][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(QueueType::Type::COMPUTE), bufferSize, 256);
	memcpy(allocation.cpuAddress, bufferData, bufferSize);
	currentCommandList_[type]->SetComputeRootShaderResourceView(rootIndex, allocation.gpuAddress);
}

void CommandContext::SetDynamicVertexBuffer(UINT slot, size_t numVertices, size_t vertexStride, const void* vertexData) {
	assert(vertexData);

	size_t bufferSize = LinearAllocator().AlignUp(numVertices * vertexStride, 256);
	auto allocation = currentDynamicBuffers_[QueueType::Type::DIRECT][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(QueueType::Type::DIRECT), bufferSize);
	memcpy(allocation.cpuAddress, vertexData, bufferSize);
	TransitionResource(QueueType::Type::DIRECT, allocation.buffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	FlushResourceBarriers();
	D3D12_VERTEX_BUFFER_VIEW vbv{
		.BufferLocation = allocation.gpuAddress,
		.SizeInBytes = UINT(bufferSize),
		.StrideInBytes = UINT(vertexStride)
	};
	currentCommandList_[QueueType::Type::DIRECT]->IASetVertexBuffers(slot, 1, &vbv);
}

void CommandContext::SetDynamicIndexBuffer(size_t numIndices, DXGI_FORMAT indexFormat, const void* indexData) {
	assert(indexData);
	assert(indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT);

	size_t stride = (indexFormat == DXGI_FORMAT_R16_UINT) ? sizeof(uint16_t) : sizeof(uint32_t);
	size_t bufferSize = LinearAllocator().AlignUp(numIndices * stride, 16);
	auto allocation = currentDynamicBuffers_[QueueType::Type::DIRECT][LinearAllocatorType::kUpload].Allocate(QueueType::GetType(QueueType::Type::DIRECT), bufferSize);
	memcpy(allocation.cpuAddress, indexData, bufferSize);
	TransitionResource(QueueType::Type::DIRECT, allocation.buffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	FlushResourceBarriers();
	D3D12_INDEX_BUFFER_VIEW ibv{
		.BufferLocation = allocation.gpuAddress,
		.SizeInBytes = UINT(numIndices * stride),
		.Format = indexFormat
	};
	currentCommandList_[QueueType::Type::DIRECT]->IASetIndexBuffer(&ibv);
}

void CommandContext::SetGraphicsConstantBuffer(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
	currentCommandList_[QueueType::Type::DIRECT]->SetGraphicsRootConstantBufferView(rootIndex, address);
}

void CommandContext::SetComputeConstantBuffer(const QueueType::Type::Param& type, UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
	currentCommandList_[type]->SetComputeRootConstantBufferView(rootIndex, address);
}

void CommandContext::SetGraphicsShaderResource(UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
	currentCommandList_[QueueType::Type::DIRECT]->SetGraphicsRootShaderResourceView(rootIndex, address);
}

void CommandContext::SetComputeShaderResource(const QueueType::Type::Param& type, UINT rootIndex, D3D12_GPU_VIRTUAL_ADDRESS address) {
	currentCommandList_[type]->SetComputeRootShaderResourceView(rootIndex, address);
}

void CommandContext::SetGraphicsDescriptorTable(UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE address) {
	currentCommandList_[QueueType::Type::DIRECT]->SetGraphicsRootDescriptorTable(rootIndex, address);
}

void CommandContext::SetComputeDescriptorTable(const QueueType::Type::Param& type, UINT rootIndex, D3D12_GPU_DESCRIPTOR_HANDLE address) {
	currentCommandList_[type]->SetComputeRootDescriptorTable(rootIndex, address);
}

void CommandContext::SetDescriptorHeaps(UINT numDescriptorHeaps, ID3D12DescriptorHeap* descriptorHeaps, const QueueType::Type::Param& type) {
	currentCommandList_[type]->SetDescriptorHeaps(numDescriptorHeaps, &descriptorHeaps);
}

void CommandContext::SetComputeUAV(const QueueType::Type::Param& type, uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation) {
	currentCommandList_[type]->SetComputeRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void CommandContext::SetGraphicsUAV(uint32_t rootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation) {
	currentCommandList_[QueueType::Type::DIRECT]->SetGraphicsRootUnorderedAccessView(rootParameterIndex, bufferLocation);
}

void CommandContext::SetVertexBuffer(UINT slot, const D3D12_VERTEX_BUFFER_VIEW& vbv) {
	currentCommandList_[QueueType::Type::DIRECT]->IASetVertexBuffers(slot, 1, &vbv);
}

void CommandContext::SetVertexBuffer(UINT slot, UINT numViews, const D3D12_VERTEX_BUFFER_VIEW vbvs[]) {
	currentCommandList_[QueueType::Type::DIRECT]->IASetVertexBuffers(slot, numViews, vbvs);
}

void CommandContext::SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& ibv) {
	currentCommandList_[QueueType::Type::DIRECT]->IASetIndexBuffer(&ibv);
}
void CommandContext::SetViewport(const D3D12_VIEWPORT& viewport) {
	currentCommandList_[QueueType::Type::DIRECT]->RSSetViewports(1, &viewport);
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
	currentCommandList_[QueueType::Type::DIRECT]->RSSetScissorRects(1, &rect);
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
	currentCommandList_[QueueType::Type::DIRECT]->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void CommandContext::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation) {
	FlushResourceBarriers();
	currentCommandList_[QueueType::Type::DIRECT]->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandContext::ExecuteIndirect(const CommandSignature& commandSignature, UINT maxCommandCount, ID3D12Resource* argumentBuffer, UINT64 argumentBufferOffset, ID3D12Resource* countBuffer, UINT64 countBufferOffset, const QueueType::Type::Param& type) {
	FlushResourceBarriers();
	ID3D12CommandSignature* cs = commandSignature;
	currentCommandList_[type]->ExecuteIndirect(cs, maxCommandCount, argumentBuffer, argumentBufferOffset, countBuffer, countBufferOffset);
}

void CommandContext::Dispatch(const QueueType::Type::Param& type, uint32_t x, uint32_t y, uint32_t z) {
	FlushResourceBarriers();
	currentCommandList_[type]->Dispatch(x, y, z);
}

void CommandContext::BeginEvent(const QueueType::Type::Param& type, const std::wstring& name) {
	type;
	name;
	//PIXBeginEvent(currentCommandList_[type].Get(), 0, name.c_str());
}

void CommandContext::EndEvent(const QueueType::Type::Param& type) {
	type;
	//PIXEndEvent(currentCommandList_[type].Get());
}

void CommandContext::SetMarker(const QueueType::Type::Param& type, const std::wstring& name) {
	type;
	name;
	//PIXSetMarker(currentCommandList_[type].Get(), 0, name.c_str());
}

D3D12_COMMAND_LIST_TYPE QueueType::GetType(const QueueType::Type::Param& type) {
	switch (type) {
	case QueueType::Type::COMPUTE:
	case QueueType::Type::SPAWN:
		return D3D12_COMMAND_LIST_TYPE_COMPUTE;
	case QueueType::Type::COPY:
		return D3D12_COMMAND_LIST_TYPE_COPY;
	default:
		break;
	}
	return D3D12_COMMAND_LIST_TYPE_DIRECT;
}

std::string QueueType::GetTypeString(const QueueType::Type::Param& type) {
	switch (type) {
	case QueueType::Type::COMPUTE:
	case QueueType::Type::SPAWN:
		return "Compute";
	case QueueType::Type::COPY:
		return "Copy";
	default:
		break;
	}
	return "Direct";
}

std::wstring QueueType::GetTypeWString(const QueueType::Type::Param& type)
{
	switch (type) {
	case QueueType::Type::COMPUTE:
	case QueueType::Type::SPAWN:
		return L"Compute";
	case QueueType::Type::COPY:
		return L"Copy";
	default:
		break;
	}
	return L"Direct";
}
