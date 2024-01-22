#include "CommandQueue.h"

#include <assert.h>
#include <chrono>
#include <thread>

#include "CommandContext.h"
#include "GraphicsCore.h"

CommandQueue::~CommandQueue() {}

void CommandQueue::Create() {
	HRESULT hr = S_FALSE;
	auto device = GraphicsCore::GetInstance()->GetDevice();

	// コマンドリストキューを作成
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(
		&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue_));
	assert(SUCCEEDED(hr));
	// フェンスを作成
	fenceValue_ = 0;
	hr = device->CreateFence(
		fenceValue_,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&fence_)
	);
	assert(SUCCEEDED(hr));
	// フェンスイベントを作成
	fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent_ != nullptr);

}

void CommandQueue::Execute(const CommandContext& commandContext) {
	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* cmdLists[] = { commandContext};
	commandQueue_->ExecuteCommandLists(_countof(cmdLists), cmdLists);
 } 

void CommandQueue::Signal() {
	commandQueue_->Signal(fence_.Get(), ++fenceValue_);
}

void CommandQueue::WaitForGPU() {
	if (fence_->GetCompletedValue() < fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
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
	WaitForGPU();
	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}
}
