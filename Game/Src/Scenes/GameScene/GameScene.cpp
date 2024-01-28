#include "GameScene.h"

#include "Engine/Audio/Audio.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"

GameScene::GameScene() {
	debugCamera_ = std::make_unique<DebugCamera>();
	gpuParticleManager_ = std::make_unique<GPUParticleManager>();

	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Ball");
	terrainHandle_ = ModelManager::GetInstance()->Load("Resources/Models/terrain");
	
	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	color_ = { 1.0f,1.0f,1.0f,1.0 };
	worldTransform_.Initialize();
	soundHandle_ = Audio::GetInstance()->SoundLoadWave("play.wav");
	playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);
	Audio::GetInstance()->SoundPlayLoopStart(playHandle_);
}

GameScene::~GameScene() {}

void GameScene::Initialize() {
	viewProjection_.Initialize();

	gpuParticleManager_->Initialize();
	EmitterForGPU emitterForGPU = {
	.min = {-5.0f,-5.0f,-5.0f},
	.maxParticleNum = 10,
	.max = {5.0f,5.0f,5.0f},
	.frequency = 30,
	.position = {0.0f,0.0f,0.0f},
	.createParticleNum = 2,
	};
	gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
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

	gpuParticleManager_->Update(RenderManager::GetInstance()->GetCommandContext());
	debugCamera_->Update(&viewProjection_);

	worldTransform_.UpdateMatrix();
	ModelManager::GetInstance()->GetModel(modelHandle_).SetMaterialColor(color_);
}

void GameScene::Draw(CommandContext& commandContext) {
	gpuParticleManager_->Draw(viewProjection_,commandContext);
	
	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection_, modelHandle_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection_, terrainHandle_, commandContext);
}

void GameScene::Finalize() {}