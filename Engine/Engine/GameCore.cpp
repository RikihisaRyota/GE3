#include "GameCore.h"

#include <memory>

#include "Graphics/RenderManager.h"
#include "Input/Input.h"
#include "Model/ModelManager.h"
#include "ShderCompiler/ShaderCompiler.h"
#include "WinApp/WinApp.h"
#include "Src/Scenes/GameScene.h"

namespace GameCore {
	Input* input=nullptr;
	RenderManager* renderManager = nullptr;
	WinApp* winApp = nullptr;
	ShaderCompiler* shaderCompiler = nullptr;
	GameScene* gameScene = nullptr;
	void Initialize() {
		winApp = WinApp::GetInstance();
		winApp->CreateGameWindow(L"LE2A_24_リキヒサ_リョウタ");

		shaderCompiler->Initialize();

		input = Input::GetInstance();
		input->Initialize();

		renderManager = RenderManager::GetInstance();
		renderManager->Initialize();
		renderManager->Reset();

		ModelManager::CreatePipeline(renderManager->GetRenderTargetFormat(),renderManager->GetDepthFormat());

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
		delete gameScene;
		renderManager->Shutdown();
		ModelManager::DestroyPipeline();
		winApp->TerminateGameWindow();
	}
}