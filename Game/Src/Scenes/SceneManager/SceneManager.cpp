#include "SceneManager.h"

#include "imgui.h"

#include "Src/Scenes/BaseScene/BaseScene.h"
#include "Src/Scenes/SceneFactory/AbstractSceneFactory/AbstractSceneFactory.h"
SceneManager::~SceneManager() {}

SceneManager* SceneManager::GetInstance() {
	static SceneManager sceneManager;
	return &sceneManager;
}

void SceneManager::Initialize(int scene, ViewProjection* viewProjection) {
	scene_ = abstractSceneFactory_->CreateScene(scene);
	scene_->SetSceneManager(this);
	scene_->SetViewProjection(viewProjection);
	scene_->Initialize();
	transition_ = std::make_unique<Transition>();
	currentScene_ = scene;

	sceneNames_.push_back("Title");
	sceneNames_.push_back("Game");
}

void SceneManager::Update(CommandContext& commandContext, ViewProjection* viewProjection) {
#ifdef ENABLE_IMGUI
	ImGui::Begin("SceneManager");
	// Combo を使用する
	if (ImGui::Combo("Scene", &currentScene_, sceneNames_.data(), static_cast<int>(sceneNames_.size()))) {
		SceneManager::GetInstance()->ChangeScene(currentScene_);
	}
	ImGui::End();
	ImGui::Begin("fps");
	ImGui::Text("Frame rate: %3.00f fps", ImGui::GetIO().Framerate);
	ImGui::Text("Delta Time: %.4f", ImGui::GetIO().DeltaTime);
	ImGui::End();
#endif // ENABLE_IMGUI

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
				scene_->SetViewProjection(viewProjection);
				scene_->Initialize();
				nextScene_ = nullptr;
			}
		}

	}
	if (transition_->GetIsTransition()) {
		transition_->Update();
	}
	if (transition_->GetIsSceneStart()) {
		scene_->Update(commandContext);
	}
}
void SceneManager::Draw(CommandContext& commandContext) {
	scene_->Draw(commandContext);
	if (transition_->GetIsTransition()) {
		transition_->Draw(commandContext);
	}
}

void SceneManager::Finalize() {
	if (scene_) {
		scene_->Finalize();
		delete scene_;
	}
	if (nextScene_) {
		nextScene_->Finalize();
		delete nextScene_;
	}
}

void SceneManager::ChangeScene(int scene) {
	nextScene_ = abstractSceneFactory_->CreateScene(scene);
}
