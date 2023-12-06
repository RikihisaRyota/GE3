#include "GameCore.h"

#include <memory>

#include "Graphics/RenderManager.h"
#include "WinApp/WinApp.h"
#include "Input/Input.h"
#include "ShderCompiler/ShaderCompiler.h"
#include "../GameScene.h"

namespace GameCore {
	Input* input=nullptr;
	RenderManager* renderManager = nullptr;
	WinApp* winApp = nullptr;
	ShaderCompiler* shaderCompiler = nullptr;
	//std::unique_ptr<GameScene> gameScene = nullptr;
	GameScene* gameScene = nullptr;
	void Initialize() {
		winApp = WinApp::GetInstance();
		winApp->CreateGameWindow(L"GE3");

		shaderCompiler->Initialize();

		input = Input::GetInstance();
		input->Initialize();

		renderManager = RenderManager::GetInstance();
		renderManager->Initialize();

		renderManager->Reset();

		//gameScene = std::make_unique<GameScene>();
		gameScene = new GameScene();
		gameScene->Initialize();

	}

	bool BeginFrame() {
		if (winApp->ProcessMessage()) {
			return false;
		}
		input->Update();

		gameScene->Update();

		renderManager->BeginRender();

		gameScene->Draw(renderManager->GetCommandContext());

		renderManager->EndRender();

		renderManager->Reset();

		return true;
	}

	void Shutdown() {
		renderManager->Shutdown();
		winApp->TerminateGameWindow();
		delete gameScene;
	}
}