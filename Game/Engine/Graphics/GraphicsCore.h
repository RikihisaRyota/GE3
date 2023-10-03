#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

#include "CommandQueue.h"

class GraphicsCore {
public: 
	static GraphicsCore* GetInstance();
public:
	void Initialize();
	void Shutdown();
public:
	ID3D12Device* GetDevice() const { return device_.Get(); }
private:
	void CreateDevice();
private:
	GraphicsCore() = default;
	GraphicsCore(const GraphicsCore&) = delete;
	GraphicsCore& operator=(const GraphicsCore&) = delete;

	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	CommandQueue commandQueue_;

};

