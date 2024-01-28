#include "SceneManager.h"

#include "Src/Scenes/BaseScene/BaseScene.h"
#include "Src/Scenes/SceneFactory/AbstractSceneFactory/AbstractSceneFactory.h"
SceneManager::~SceneManager() {
	scene_->Finalize();
	delete scene_;
}

SceneManager* SceneManager::GetInstance() {
	static SceneManager sceneManager;
	return &sceneManager;
}

void SceneManager::Initialize(AbstractSceneFactory::Scene scene) {
	scene_ = abstractSceneFactory_->CreateScene(scene);
	scene_->SetSceneManager(this);
	scene_->Initialize();
}

void SceneManager::Update() {
	scene_->Update();
	if (nextScene_) {
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}
		scene_->SetSceneManager(this);
		scene_ = nextScene_;
		nextScene_ = nullptr;
		scene_->Initialize();
	}
}
void SceneManager::Draw(CommandContext& commandContext) {
	scene_->Draw(commandContext);
}

void SceneManager::ChangeScene(AbstractSceneFactory::Scene scene) {
	nextScene_ = abstractSceneFactory_->CreateScene(scene);
}
