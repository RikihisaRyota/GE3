#include "GameCore.h"

#include <memory>

#include "Audio/Audio.h"
#include "Graphics/RenderManager.h"
#include "Input/Input.h"
#include "Model/ModelManager.h"
#include "ShderCompiler/ShaderCompiler.h"
#include "Src/Scenes/SceneManager/SceneManager.h"
#include "Src/Scenes/SceneFactory/SceneFactory.h"
#include "WinApp/WinApp.h"

namespace GameCore {
	Audio* audio = nullptr;
	Input* input = nullptr;
	RenderManager* renderManager = nullptr;
	WinApp* winApp = nullptr;
	ShaderCompiler* shaderCompiler = nullptr;
	SceneManager* sceneManager = nullptr;
	SceneFactory* sceneFactory = nullptr;

	void Initialize() {
		winApp = WinApp::GetInstance();
		winApp->CreateGameWindow(L"LE2A_24_リキヒサ_リョウタ");

		shaderCompiler->Initialize();

		audio = Audio::GetInstance();
		audio->Initialize();

		input = Input::GetInstance();
		input->Initialize();

		renderManager = RenderManager::GetInstance();
		renderManager->Initialize();
		renderManager->Reset();

		ModelManager::CreatePipeline(renderManager->GetRenderTargetFormat(),renderManager->GetDepthFormat());

		sceneManager = SceneManager::GetInstance();

		sceneFactory = new SceneFactory();
		sceneManager->SetSceneFactory(sceneFactory);
		sceneManager->Initialize(AbstractSceneFactory::Scene::kGame);
	}

	bool BeginFrame() {
		if (winApp->ProcessMessage()) {
			return false;
		}
		input->Update();

		audio->Update();

		sceneManager->Update();

		renderManager->BeginRender();

		sceneManager->Draw(renderManager->GetCommandContext());

		renderManager->EndRender();

		renderManager->Reset();

		return true;
	}

	void Shutdown() {
		delete sceneFactory;
		renderManager->Shutdown();
		ModelManager::DestroyPipeline();
		winApp->TerminateGameWindow();
	}
}