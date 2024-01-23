#include "GameScene.h"

#include "Engine/Audio/Audio.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Model/ModelManager.h"

#include "Engine/ImGui/ImGuiManager.h"

GameScene::GameScene() {
	viewProjection_.Initialize();
	gpuParticle_ = std::make_unique<GPUParticle>();
	debugCamera_ = std::make_unique<DebugCamera>();
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Ball");
	terrainHandle_ = ModelManager::GetInstance()->Load("Resources/Models/terrain");
	color_ = { 1.0f,1.0f,1.0f,1.0 };
	worldTransform_.Initialize();
	soundHandle_ = Audio::GetInstance()->SoundLoadWave("play.wav");
	playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);
	Audio::GetInstance()->SoundPlayLoopStart(playHandle_);
}

GameScene::~GameScene() {}

void GameScene::Initialize() {
	gpuParticle_->Initialize();
}

void GameScene::Update() {
#ifdef _DEBUG
	ImGui::Begin("fps");
	ImGui::Text("Frame rate: %3.0f fps", ImGui::GetIO().Framerate);
	ImGui::Text("Delta Time: %.4f", ImGui::GetIO().DeltaTime);
	ImGui::End();
	ImGui::Begin("Material");
	ImGui::DragFloat4("color", &color_.x, 0.01f, 0.0f, 1.0f);
	ImGui::End();
#endif // _DEBUG

	gpuParticle_->Update(&viewProjection_);
	debugCamera_->Update(&viewProjection_);

	worldTransform_.UpdateMatrix();
	ModelManager::GetInstance()->GetModel(modelHandle_).SetMaterialColor(color_);
}

void GameScene::Draw(CommandContext& commandContext) {
	gpuParticle_->Render(viewProjection_);
	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection_, modelHandle_, commandContext);
	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection_, terrainHandle_, commandContext);
}

void GameScene::Finalize() {}
