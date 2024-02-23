#include "GameScene.h"

#include "Engine/Audio/Audio.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine//Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Sprite/SpriteManager.h"

GameScene::GameScene() {
	debugCamera_ = std::make_unique<DebugCamera>();
	gpuParticleManager_ = std::make_unique<GPUParticleManager>();
	player_ = std::make_unique<Player>();
	followCamera_ = std::make_unique<FollowCamera>();

	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Ball");
	terrainHandle_ = ModelManager::GetInstance()->Load("Resources/Models/terrain");

	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	color_ = { 1.0f,1.0f,1.0f,1.0 };
	/*soundHandle_ = Audio::GetInstance()->SoundLoadWave("play.wav");
	playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);*/

}

GameScene::~GameScene() {}

void GameScene::Initialize() {
	//Audio::GetInstance()->SoundPlayLoopStart(playHandle_);
	worldTransform_.Initialize();


	player_->SetViewProjection(viewProjection_);
	player_->Initialize();

	followCamera_->SetTarget(&player_->GetWorldTransform());
	followCamera_->SetViewProjection(viewProjection_);
	followCamera_->Initialize();
	gpuParticleManager_->Initialize();
	{
		EmitterForGPU emitterForGPU = {
		.min = {-0.5f,-1.0f,-0.5f},
		.createParticleNum = 1 << 10,
		.max = {0.5f,1.0f,0.5f},
		.isAlive = true,
		.position = {0.0,0.0f,0.0f},
		.time = 0,
		.interval = 0,
		.isLoop = true,
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
	{
		EmitterForGPU emitterForGPU = {
		.min = {-0.5f,-1.0f,-0.5f},
		.createParticleNum = 1 << 10,
		.max = {0.5f,1.0f,0.5f},
		.isAlive = true,
		.position = {2.0,0.0f,0.0f},
		.time = 0,
		.interval = 0,
		.isLoop = true,
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
	/*{
		EmitterForGPU emitterForGPU = {
		.min = {-0.5f,-1.0f,-0.5f},
		.createParticleNum = 1 << 12,
		.max = {0.5f,1.0f,0.5f},
		.isAlive = true,
		.position = {-2.0,0.0f,0.0f},
		.time = 0,
		.interval = 120,
		.isLoop = true,
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
	{
		EmitterForGPU emitterForGPU = {
		.min = {-0.5f,-1.0f,-0.5f},
		.createParticleNum = 1 << 13,
		.max = {0.5f,1.0f,0.5f},
		.isAlive = true,
		.position = {0.0,2.0f,0.0f},
		.time = 0,
		.interval = 120,
		.isLoop = true,
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}
	
	{
		EmitterForGPU emitterForGPU = {
		.min = {-0.5f,-1.0f,-0.5f},
		.createParticleNum = 1 << 14,
		.max = {0.5f,1.0f,0.5f},
		.isAlive = true,
		.position = {0.0,-2.0f,0.0f},
		.time = 0,
		.interval = 120,
		.isLoop = true,
		};
		gpuParticleManager_->CreateParticle(emitterForGPU, gpuTexture_);
	}*/
}

void GameScene::Update() {

#ifdef ENABLE_IMGUI
	ImGui::Begin("Sphere");
	ImGui::DragFloat3("scale", &worldTransform_.scale_.x, 0.1f, 0.0f);
	ImGui::End();
#endif // ENABLE_IMGUI
	gpuParticleManager_->Update(RenderManager::GetInstance()->GetCommandContext());

	debugCamera_->Update(viewProjection_);

	if (!debugCamera_->GetIsDebugCamera()) {
		followCamera_->Update();
	}
	player_->Update();

	worldTransform_.UpdateMatrix();
	ModelManager::GetInstance()->GetModel(modelHandle_).SetMaterialColor(color_);
}

void GameScene::Draw(CommandContext& commandContext) {
	gpuParticleManager_->Draw(*viewProjection_, commandContext);

	/*player_->Draw(*viewProjection_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, *viewProjection_, modelHandle_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, *viewProjection_, terrainHandle_, commandContext);*/
}

void GameScene::Finalize() {}
