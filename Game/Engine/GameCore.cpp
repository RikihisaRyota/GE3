#include "GameCore.h"

namespace GameCore {
	Input* input=nullptr;
	RenderManager* renderManager = nullptr;
	WinApp* winApp = nullptr;
	void Initialize() {
		winApp = WinApp::GetInstance();
		winApp->CreateGameWindow(L"GE3");

		input = Input::GetInstance();
		input->Initialize();

		renderManager = RenderManager::GetInstance();
		renderManager->Initialize();

		renderManager->Reset();
	}

	bool BeginFrame() {
		if (winApp->ProcessMessage()) {
			return false;
		}
		input->Update();

		renderManager->BeginRender();

		renderManager->EndRender();

		renderManager->Reset();

		return true;
	}

	void Shutdown() {
		renderManager->Shutdown();
		winApp->TerminateGameWindow();
	}
}