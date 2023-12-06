#include "GameScene.h"

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"

GameScene::GameScene() {
	gpuParticle_ = std::make_unique<GPUParticle>();
}

void GameScene::Initialize() {
	gpuParticle_->Initialize();
}

void GameScene::Update() {
	gpuParticle_->Update();
}

void GameScene::Draw(const CommandContext& commandContext) {
}
