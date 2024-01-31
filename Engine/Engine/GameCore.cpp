#include "GameCore.h"

#include <memory>

#include "Audio/Audio.h"
#include "Engine/Lighting/Lighting.h"
#include "Graphics/RenderManager.h"
#include "Input/Input.h"
#include "Model/ModelManager.h"
#include "ShderCompiler/ShaderCompiler.h"
#include "Src/Scenes/SceneManager/SceneManager.h"
#include "Src/Scenes/SceneFactory/SceneFactory.h"
#include "WinApp/WinApp.h"
#include "Sprite/SpriteManager.h"

namespace GameCore {
	Audio* audio = nullptr;
	Input* input = nullptr;
	RenderManager* renderManager = nullptr;
	WinApp* winApp = nullptr;
	ShaderCompiler* shaderCompiler = nullptr;
	SceneManager* sceneManager = nullptr;
	SceneFactory* sceneFactory = nullptr;
	Lighting* lighting = nullptr;
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

		ModelManager::CreatePipeline(renderManager->GetRenderTargetFormat(), renderManager->GetDepthFormat());

		SpriteManager::CreatePipeline(renderManager->GetRenderTargetFormat(), renderManager->GetDepthFormat());

		sceneManager = SceneManager::GetInstance();

		sceneFactory = new SceneFactory();
		sceneManager->SetSceneFactory(sceneFactory);
		sceneManager->Initialize(AbstractSceneFactory::Scene::kTitle);

		lighting = Lighting::GetInstance();
		lighting->Initialize();
	}

	bool BeginFrame() {
		if (winApp->ProcessMessage()) {
			return false;
		}
		input->Update();

		audio->Update();

		lighting->Update();

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
		SpriteManager::DestroyPipeline();
		ModelManager::DestroyPipeline();
		winApp->TerminateGameWindow();
	}
}