#include "GameCore.h"

namespace GameCore {
	Input* input=nullptr;
	RenderManager* renderManager = nullptr;
	WinApp* winApp = nullptr;
	ShaderCompiler* shaderCompiler = nullptr;
	GameScene* gameScene = nullptr;
	void Initialize() {
		winApp = WinApp::GetInstance();
		winApp->CreateGameWindow(L"GE3");

		shaderCompiler->Initialize();

		input = Input::GetInstance();
		input->Initialize();

		renderManager = RenderManager::GetInstance();
		renderManager->Initialize();

		gameScene = new GameScene();
		gameScene->Initialize();

		renderManager->Reset();
	}

	bool BeginFrame() {
		if (winApp->ProcessMessage()) {
			return false;
		}
		input->Update();

		gameScene->Update();

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