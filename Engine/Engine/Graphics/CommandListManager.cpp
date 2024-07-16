#include "CommandListManager.h"

#include <assert.h>
#include <thread>

#include "CommandContext.h"
#include "GraphicsCore.h"
#include "CommandListManager.h"

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE Type) :
	type_(Type),
	commandQueue_(nullptr),
	fence_(nullptr),
	nextFenceValue_((uint64_t)Type << 56 | 1),
	lastCompletedFenceValue_((uint64_t)Type << 56),
	allocatorPool_(Type) {}

CommandQueue::~CommandQueue() {
	Shutdown();
}

void CommandQueue::Create() {
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

	// フェンスを作成
	hr = device->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&fence_)
	);
	assert(SUCCEEDED(hr));
	fence_->SetName(L"Fence");
	fence_->Signal((uint64_t)type_ << 56);

	// フェンスイベントを作成
	fenceEventHandle_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEventHandle_ != nullptr);

	assert(IsReady());
}

uint64_t CommandQueue::IncrementFence(void) {
	std::lock_guard<std::mutex> LockGuard(fenceMutex_);
	commandQueue_->Signal(fence_.Get(), nextFenceValue_);
	return nextFenceValue_++;
}

bool CommandQueue::IsFenceComplete(uint64_t FenceValue) {
	if (FenceValue > lastCompletedFenceValue_)
		lastCompletedFenceValue_ = (std::max)(lastCompletedFenceValue_, fence_->GetCompletedValue());

	return FenceValue <= lastCompletedFenceValue_;
}

void CommandQueue::StallForFence(uint64_t FenceValue) {
	CommandQueue& Producer = CommandListManager().GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	commandQueue_->Wait(Producer.fence_.Get(), FenceValue);
}

void CommandQueue::StallForProducer(CommandQueue& Producer) {
	assert(Producer.nextFenceValue_ > 0);
	commandQueue_->Wait(Producer.fence_.Get(), Producer.nextFenceValue_ - 1);
}

void CommandQueue::WaitForFence(uint64_t FenceValue) {
	if (IsFenceComplete(FenceValue)) {
		return;
	}
	{
		std::lock_guard<std::mutex> LockGuard(eventMutex_);

		fence_->SetEventOnCompletion(FenceValue, fenceEventHandle_);
		WaitForSingleObject(fenceEventHandle_, INFINITE);
		lastCompletedFenceValue_ = FenceValue;
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

void CommandQueue::Shutdown() {
	if (commandQueue_ == nullptr)
		return;
	WaitForIdle();
	//allocatorPool_.Shutdown();

	CloseHandle(fenceEventHandle_);
	fenceEventHandle_ = nullptr;
	//fence_->Release();
	//fence_ = nullptr;

	//commandQueue_->Release();
	//commandQueue_ = nullptr;
}

uint64_t CommandQueue::ExecuteCommandList(ID3D12CommandList* List) {
	std::lock_guard<std::mutex> LockGuard(fenceMutex_);

	commandQueue_->ExecuteCommandLists(1, &List);

	commandQueue_->Signal(fence_.Get(), nextFenceValue_);

	return nextFenceValue_++;
}

Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueue::RequestAllocator(void) {
	uint64_t CompletedFence = fence_->GetCompletedValue();

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
	graphicsQueue_.Shutdown();
	computeQueue_.Shutdown();
	copyQueue_.Shutdown();
}

void CommandListManager::Create() {

	graphicsQueue_.Create();
	computeQueue_.Create();
	copyQueue_.Create();
}

void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator) {
	assert(Type != D3D12_COMMAND_LIST_TYPE_BUNDLE);
	switch (Type) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT: *Allocator = graphicsQueue_.RequestAllocator().Get(); break;
	case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE: *Allocator = computeQueue_.RequestAllocator().Get(); break;
	case D3D12_COMMAND_LIST_TYPE_COPY: *Allocator = copyQueue_.RequestAllocator().Get(); break;
	}
	assert(SUCCEEDED(GraphicsCore::GetInstance()->GetDevice()->CreateCommandList(1, Type, *Allocator, nullptr, IID_PPV_ARGS(List))));
	(*List)->SetName(L"CommandList");
}

void CommandListManager::WaitForFence(uint64_t FenceValue) {
	CommandQueue& Producer = GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
	Producer.WaitForFence(FenceValue);
}
