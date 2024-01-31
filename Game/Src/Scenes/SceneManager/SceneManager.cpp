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
	transition_ = std::make_unique<Transition>();
}

void SceneManager::Update() {
	if (nextScene_) {
		if (!transition_->GetIsTransition()) {
			transition_->Initialize();
		}
		else {
			if (transition_->GetIsSceneChange()) {
				if (scene_) {
					scene_->Finalize();
					delete scene_;
				}
				scene_ = nextScene_;
				scene_->SetSceneManager(this);
				scene_->Initialize();
				nextScene_ = nullptr;
			}
		}

	}
	if (transition_->GetIsTransition()) {
		transition_->Update();
	}
	if (transition_->GetIsSceneStart()) {
		scene_->Update();
	}
}
void SceneManager::Draw(CommandContext& commandContext) {
	scene_->Draw(commandContext);
	if (transition_->GetIsTransition()) {
		transition_->Draw(commandContext);
	}
}

void SceneManager::ChangeScene(AbstractSceneFactory::Scene scene) {
	nextScene_ = abstractSceneFactory_->CreateScene(scene);
}
