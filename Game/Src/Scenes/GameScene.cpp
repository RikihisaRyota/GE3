#include "GameScene.h"

#include "../../Engine/Graphics/CommandContext.h"
#include "../../Engine/Graphics/RenderManager.h"

GameScene::GameScene() {
	viewProjection_.Initialize();
	gpuParticle_ = std::make_unique<GPUParticle>();
	debugCamera_ = std::make_unique<DebugCamera>();
}

void GameScene::Initialize() {
	gpuParticle_->Initialize();
}

void GameScene::Update() {
	gpuParticle_->Update();
	debugCamera_->Update(&viewProjection_);
}

void GameScene::Draw(const CommandContext& commandContext) {
	gpuParticle_->Render(viewProjection_);
}
