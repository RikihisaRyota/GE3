#include "GameScene.h"

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"

GameScene::GameScene() {}

void GameScene::Initialize() {
	gpuParticle_.Initialize();
}

void GameScene::Update() {
	gpuParticle_.Update(RenderManager::GetInstance()->GetCommandContext());
}

void GameScene::Draw(const CommandContext& commandContext) {
}
