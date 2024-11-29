#include "CommandListManager.h"

#include <assert.h>
#include <thread>

#include <sstream>
#include <iomanip>

#include "Engine/ConvertString/ConvertString.h"
#include "CommandContext.h"
#include "GraphicsCore.h"
#include "CommandListManager.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE Type)
	: type_(Type), commandQueue_(nullptr), allocatorPool_(Type) {
	lastCompletedFenceValue_ = 0;
}

CommandQueue::~CommandQueue() {
	//Shutdown();
}

void CommandQueue::Create(const std::wstring& name) {
	HRESULT hr = S_FALSE;
	auto device = GraphicsCore::GetInstance()->GetDevice();
	// コマンドリストキューを作成
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	commandQueueDesc.Type = type_;
	commandQueueDesc.NodeMask = 1;

	hr = device->CreateCommandQueue(
		&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr));
	commandQueue_->SetName(name.c_str());

	assert(IsReady());
}

uint64_t CommandQueue::IncrementFence(ID3D12Fence* fence) {
	std::lock_guard<std::mutex> LockGuard(fenceMutex_);
	commandQueue_->Signal(fence, ++currentSignalValue_);
	return currentSignalValue_;
}

void CommandQueue::StallForFence(ID3D12Fence* fence, uint64_t FenceValue) {
	CommandQueue& Producer = CommandListManager().GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	commandQueue_->Wait(fence, FenceValue);
}

bool CommandQueue::IsFenceComplete(ID3D12Fence* fence, uint64_t FenceValue) {
	if (FenceValue > lastCompletedFenceValue_)
		lastCompletedFenceValue_ = (std::max)(lastCompletedFenceValue_, fence->GetCompletedValue());

	return FenceValue <= lastCompletedFenceValue_;
}

void CommandQueue::WaitForFence(ID3D12Fence* fence, const HANDLE& handle, uint64_t FenceValue) {
	if (IsFenceComplete(fence, FenceValue)) {
		std::string s;
		switch (type_) {
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			s = "DIRECT";
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			s = "COMPUTE";
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			s = "COPY";
			break;
		default:
			break;
		}
		std::string t;

		std::stringstream ss;
		ss
			<< std::setw(10) << std::left << s
			<< std::setw(15) << std::left << "CompleteFence"
			<< std::setw(15) << std::left << "currentValue : " << std::to_string(FenceValue)
			<< std::setw(10) << std::left << "lastCompletedFenceValue : " << lastCompletedFenceValue_;
		Log(ss.str() + "\n");
		return;
	}
	{
		std::lock_guard<std::mutex> LockGuard(eventMutex_);

		fence->SetEventOnCompletion(FenceValue, handle);
		WaitForSingleObject(handle, INFINITE);
		lastCompletedFenceValue_ = FenceValue;
		std::string s;
		switch (type_) {
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			s = "DIRECT";
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			s = "COMPUTE";
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			s = "COPY";
			break;
		default:
			break;
		}

		std::stringstream ss;
		ss
			<< std::setw(10) << std::left << s
			<< std::setw(15) << std::left << "CompleteFence"
			<< std::setw(15) << std::left << "currentValue : " << std::to_string(FenceValue)
			<< std::setw(10) << std::left << "lastCompletedFenceValue : " << lastCompletedFenceValue_;
		Log(ss.str() + "\n");
	}
}

void CommandQueue::UpdateFixFPS() {
	// FPS固定
	{
		timeBeginPeriod(1);
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		std::chrono::microseconds elapsed =
			std::chrono::duration_cast<std::chrono::microseconds>(now - referenceTime_);

		static const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 62.0f));
		static const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
		std::chrono::microseconds check = kMinCheckTime - elapsed;
		if (check > std::chrono::microseconds(0)) {
			std::chrono::microseconds waitTime = kMinTime - elapsed;

			std::chrono::steady_clock::time_point waitStart = std::chrono::steady_clock::now();
			do {
				std::this_thread::sleep_for(std::chrono::nanoseconds(1));
			} while (std::chrono::steady_clock::now() - waitStart < waitTime);
		}

		elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::steady_clock::now() - referenceTime_);
		referenceTime_ = std::chrono::steady_clock::now();
		timeBeginPeriod(1);
	}
}

void CommandQueue::Shutdown(ID3D12Fence* fence, const HANDLE& handle) {
	if (commandQueue_ == nullptr)
		return;
	WaitForIdle(fence, handle);
	//allocatorPool_.Shutdown();

	//fence_->Release();
	//fence_ = nullptr;

	//commandQueue_->Release();
	//commandQueue_ = nullptr;
}

void CommandQueue::ExecuteCommandList(ID3D12CommandList* List, const std::string& fenceType) {
	std::lock_guard<std::mutex> LockGuard(fenceMutex_);

	commandQueue_->ExecuteCommandLists(1, &List);

	std::stringstream ss;
	ss << fenceType
		<< std::setw(15) << std::left << "ExeCute ";
	Log(ss.str() + "\n");
}

void CommandQueue::Wait(ID3D12Fence* fence, uint64_t& value, const std::string& fenceType) {
	commandQueue_->Wait(fence, value);
	std::stringstream ss;
	ss << fenceType
		<< std::setw(15) << std::left << "Wait "
		<< std::setw(10) << std::left << "value :"
		<< std::to_string(value); 
	Log(ss.str() + "\n");
}

void CommandQueue::Signal(ID3D12Fence* fence, uint64_t& value, const std::string& fenceType) {
	commandQueue_->Signal(fence, ++value);
	std::stringstream ss;
	ss << fenceType
		<< std::setw(15) << std::left << "Signal"
		<< std::setw(10) << std::left << "value :"
		<< std::to_string(value);
	Log(ss.str() + "\n");

}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueue::RequestAllocator(ID3D12Fence* fence) {
	uint64_t CompletedFence = fence->GetCompletedValue();

	return allocatorPool_.Allocate(CompletedFence);
}

void CommandQueue::DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator) {
	allocatorPool_.Discard(FenceValueForReset, Allocator);
}

CommandListManager::CommandListManager() :
	graphicsQueue_(D3D12_COMMAND_LIST_TYPE_DIRECT),
	computeQueue_(D3D12_COMMAND_LIST_TYPE_COMPUTE),
	copyQueue_(D3D12_COMMAND_LIST_TYPE_COPY) {}

CommandListManager::~CommandListManager() {
	Shutdown();
}

void CommandListManager::Shutdown() {
	graphicsQueue_.Shutdown(frameFenceDesc_.fence.Get(), frameFenceDesc_.fenceEventHandle);
	computeQueue_.Shutdown(frameFenceDesc_.fence.Get(), frameFenceDesc_.fenceEventHandle);
	copyQueue_.Shutdown(frameFenceDesc_.fence.Get(), frameFenceDesc_.fenceEventHandle);
	for (uint32_t i = 0; i < QueueType::Type::Param::COUNT; i++) {
		graphicsQueue_.Shutdown(queueFenceDesc_[i].fence.Get(), queueFenceDesc_[i].fenceEventHandle);
		computeQueue_.Shutdown(queueFenceDesc_[i].fence.Get(), queueFenceDesc_[i].fenceEventHandle);
		copyQueue_.Shutdown(queueFenceDesc_[i].fence.Get(), queueFenceDesc_[i].fenceEventHandle);

		CloseHandle(queueFenceDesc_[i].fenceEventHandle);
		CloseHandle(frameFenceDesc_.fenceEventHandle);
		queueFenceDesc_[i].fenceEventHandle = nullptr;
		frameFenceDesc_.fenceEventHandle = nullptr;
	}
}

void CommandListManager::Create() {
	graphicsQueue_.Create(L"DIRECT");
	computeQueue_.Create(L"COMPUTE");
	copyQueue_.Create(L"COPY");
	CreateFence();
}

//void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator) {
//	assert(Type != D3D12_COMMAND_LIST_TYPE_BUNDLE);
//	switch (Type) {
//	case D3D12_COMMAND_LIST_TYPE_DIRECT: *Allocator = graphicsQueue_.RequestAllocator(fenceDesc_.fence.Get()).Get(); break;
//	case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
//	case D3D12_COMMAND_LIST_TYPE_COMPUTE: *Allocator = computeQueue_.RequestAllocator(fenceDesc_.fence.Get()).Get(); break;
//	case D3D12_COMMAND_LIST_TYPE_COPY: *Allocator = copyQueue_.RequestAllocator(fenceDesc_.fence.Get()).Get(); break;
//	}
//	assert(SUCCEEDED(GraphicsCore::GetInstance()->GetDevice()->CreateCommandList(1, Type, *Allocator, nullptr, IID_PPV_ARGS(List))));
//	(*List)->SetName(L"CommandList");
//}

//void CommandListManager::WaitForFence(uint64_t FenceValue) {
//	CommandQueue& Producer = GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
//	Producer.WaitForFence(fenceDesc_.fence.Get(), fenceDesc_.fenceEventHandle, fenceDesc_.fenceValue);
//}

void CommandListManager::CreateFence() {
	auto device = GraphicsCore::GetInstance()->GetDevice();
	for (uint32_t i = 0; i < QueueType::Type::COUNT + 1; i++) {
		if (i < QueueType::Type::COUNT) {
			queueFenceDesc_[i].fenceValue = 0;
			// フェンスを作成
			auto hr = device->CreateFence(
				0,
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&queueFenceDesc_[i].fence)
			);
			assert(SUCCEEDED(hr));

			// フェンスイベントを作成
			queueFenceDesc_[i].fenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(queueFenceDesc_[i].fenceEventHandle != nullptr);
			queueFenceDesc_[i].fence->Signal(queueFenceDesc_[i].fenceValue);
		}
		else {
			frameFenceDesc_.fenceValue = 0;
			// フェンスを作成
			auto hr = device->CreateFence(
				0,
				D3D12_FENCE_FLAG_NONE,
				IID_PPV_ARGS(&frameFenceDesc_.fence)
			);
			assert(SUCCEEDED(hr));

			// フェンスイベントを作成
			frameFenceDesc_.fenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			assert(frameFenceDesc_.fenceEventHandle != nullptr);
			frameFenceDesc_.fence->Signal(frameFenceDesc_.fenceValue);
		}
	}
	queueFenceDesc_[QueueType::Type::COPY].fence->SetName(L"CopyQueueFence");
	queueFenceDesc_[QueueType::Type::COMPUTE].fence->SetName(L"ComputeQueueFence");
	queueFenceDesc_[QueueType::Type::DIRECT].fence->SetName(L"DirectQueueFence");
	frameFenceDesc_.fence->SetName(L"FrameQueueFence");
}
