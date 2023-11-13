#include <cassert>
#include <dxgidebug.h>
#include <Windows.h>

#include "Engine/GameCore.h"

#pragma comment(lib,"dxguid.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	
	GameCore::Initialize();

	// メインループ
	while (GameCore::BeginFrame()) {
		

	}

	GameCore::Shutdown();

	// リリースリークチェック
	/*IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}*/
	return 0;
}