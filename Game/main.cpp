#include <cassert>
#include <dxgidebug.h>
#include <Windows.h>

#include "Engine/WinApp/WinApp.h"


#pragma comment(lib,"dxguid.lib")

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#pragma region ポインタ置き場
	WinApp* win = nullptr;
#pragma endregion
#pragma region ゲームウィンドウ作成
	win = WinApp::GetInstance();
	win->CreateGameWindow(L"GE3");
#pragma endregion
#pragma region DirectX初期化

#pragma endregion
#pragma region 汎用機能初期化

#pragma endregion
#pragma region シーンの初期化

#pragma endregion
	// メインループ
	while (true) {
		// メッセージ処理
		if (win->ProcessMessage()) {
			break;
		}
	}

#pragma region 解放処理
	// WindosAPI
	win->TerminateGameWindow();
#pragma endregion

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