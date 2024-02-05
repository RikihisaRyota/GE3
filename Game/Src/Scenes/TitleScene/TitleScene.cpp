#include "TitleScene.h"

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine/Input/Input.h"
#include "Engine/Texture/TextureManager.h"
#include "Src/Scenes/SceneManager/SceneManager.h"
#include "Engine/Sprite/SpriteManager.h"

TitleScene::TitleScene() {
	debugCamera_ = std::make_unique<DebugCamera>();
	gpuParticleManager_ = std::make_unique<GPUParticleManager>();
	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	titleTexture_ = TextureManager::GetInstance()->Load("Resources/Images/title.png");
	titleHandle_ = SpriteManager::GetInstance()->Create(titleTexture_, { 1280.0f * 0.5f,720.0f * 0.5f }, { 0.5f,0.5f });

}

TitleScene::~TitleScene() {}

void TitleScene::Initialize() {
	viewProjection_.Initialize();
	gpuParticleManager_->Initialize();
	{
		EmitterForGPU emitterForGPU = {
		.min = {-30.0f,-30.0f,-10.0f},
		.maxParticleNum = 1 << 24,
		.max = {30.0f,30.0f,10.0f},
		.createParticleNum = 1 << 16,
		.position = {0.0,0.0f,50.0f},
		};

		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
}

void TitleScene::Update() {
	viewProjection_.UpdateMatrix();
	gpuParticleManager_->Update(RenderManager::GetInstance()->GetCommandContext());
	debugCamera_->Update(&viewProjection_);
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChangeScene(AbstractSceneFactory::Scene::kGame);
	}
}

void TitleScene::Draw(CommandContext& commandContext) {
	gpuParticleManager_->Draw(viewProjection_, commandContext);
	SpriteManager::GetInstance()->Draw(titleHandle_, commandContext);
}

void TitleScene::Finalize() {}
