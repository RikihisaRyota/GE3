#include "CommandQueue.h"

#include <assert.h>

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

void CommandQueue::Excute() {
	// GPUにコマンドリストの実行を行わせる
	//ID3D12CommandList* cmdLists[] = { commandList_.Get() };
	//commandQueue_->ExecuteCommandLists(1, cmdLists);
}

void CommandQueue::Signal() {
	commandQueue_->Signal(fence_.Get(), ++fenceValue_);
}

void CommandQueue::WaitForGPU() {
	if (fence_->GetCompletedValue() != fenceValue_) {
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}

void CommandQueue::Shutdown() {
	WaitForGPU();
	if (fenceEvent_) {
		CloseHandle(fenceEvent_);
		fenceEvent_ = nullptr;
	}
}
