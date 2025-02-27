#include "GameCore.h"

#include <memory>

#include "Audio/Audio.h"
#include "DrawLine/DrawLine.h"
#include "Engine/Lighting/Lighting.h"
#include "Engine/ParticleManager/ParticleManager.h"
#include "Graphics/RenderManager.h"
#include "Input/Input.h"
#include "Model/ModelManager.h"
#include "ShderCompiler/ShaderCompiler.h"
#include "Src/Scenes/SceneManager/SceneManager.h"
#include "Src/Scenes/SceneFactory/SceneFactory.h"
#include "WinApp/WinApp.h"
#include "Sprite/SpriteManager.h"
#include "Animation/Skinning.h"

namespace GameCore {
	Engine::Audio* audio = nullptr;
	Engine::Input* input = nullptr;
	RenderManager* renderManager = nullptr;
	WinApp* winApp = nullptr;
	ShaderCompiler* shaderCompiler = nullptr;
	SceneManager* sceneManager = nullptr;
	SceneFactory* sceneFactory = nullptr;
	Lighting* lighting = nullptr;
	ParticleManager* particleManager = nullptr;
	ViewProjection* viewProjection = nullptr;
	DrawLine* drawLine = nullptr;
	void Initialize() {
		winApp = WinApp::GetInstance();
		winApp->CreateGameWindow(L"シューティングバスター");

		shaderCompiler->Initialize();

		audio = Engine::Audio::GetInstance();
		audio->Initialize();

		input = Engine::Input::GetInstance();
		input->Initialize();

		renderManager = RenderManager::GetInstance();
		renderManager->Initialize();
		renderManager->Reset();

		ModelManager::CreatePipeline(renderManager->GetRenderTargetFormat(), renderManager->GetDepthFormat());

		SpriteManager::CreatePipeline(renderManager->GetRenderTargetFormat(), renderManager->GetDepthFormat());

		viewProjection = new ViewProjection();
		viewProjection->Initialize();

		drawLine = DrawLine::GetInstance();
		drawLine->Initialize();

		sceneManager = SceneManager::GetInstance();
		sceneFactory = new SceneFactory();
		sceneManager->SetSceneFactory(sceneFactory);
		sceneManager->Initialize(AbstractSceneFactory::Scene::kGame,viewProjection);

		lighting = Lighting::GetInstance();
		lighting->Initialize();

		particleManager = ParticleManager::GetInstance();
		particleManager->Initialize();

		Engine::Animation::Initialize();
	}

	bool BeginFrame() {
		if (winApp->ProcessMessage()) {
			return false;
		}
		renderManager->BeginRender();

		input->Update();

		audio->Update();

		lighting->Update();

		sceneManager->Update(renderManager->GetCommandContext(),viewProjection);

		particleManager->Update();

		viewProjection->UpdateMatrix();

		renderManager->BeginDraw();

		sceneManager->Draw(renderManager->GetCommandContext());

		particleManager->Draw(renderManager->GetCommandContext(),*viewProjection);

		drawLine->Draw(renderManager->GetCommandContext(), *viewProjection);
		
		renderManager->EndRender(*viewProjection);

		//renderManager->Reset();

		return true;
	}

	void Shutdown() {
		Engine::Animation::Release();
		particleManager->Shutdown();
		sceneManager->Finalize();
		delete viewProjection;
		delete sceneFactory;
		renderManager->Shutdown();
		SpriteManager::DestroyPipeline();
		ModelManager::DestroyPipeline();
		winApp->TerminateGameWindow();
	}
}