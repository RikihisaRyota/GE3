#include <d3d12.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <cassert>
#include <Windows.h>
#include <wrl.h>

#include "Engine/GameCore.h"

#pragma comment(lib,"dxguid.lib")

struct ResourceLeakChecker {
	~ResourceLeakChecker() {
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		}
	}
};

static ResourceLeakChecker leakCheck;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameCore::Initialize();

	// メインループ
	while (GameCore::BeginFrame()) {
	}

	GameCore::Shutdown();
	return 0;
}