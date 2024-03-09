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
	// 0
	{
		Emitter emitterForGPU = {
		.emitterArea{
				.area{
					.min = {-5.0f,-5.0f,-5.0f},
					.max = {5.0f,5.0f,5.0f},
				},
				.position = {0.0f,0.0f,0.0f},
			},

		.scale{
			.range{
				.start{
					.min = {0.1f,0.1f,0.1f},
					.max = {0.2f,0.2f,0.2f},
				},
				.end{
					.min = {0.4f,0.4f,0.4f},
					.max = {0.6f,0.6f,0.6f},
				},
			},
		},

		.rotate{
			.rotate = {0.0f,0.0f,0.3f},
		},

		.velocity{
			.velocity = {0.0f,-0.1f,0.0f},
		},

		.color{
			.range{
				.start{
					.min = {0.8f,0.2f,0.2f,1.0f},
					.max = {1.0f,0.5f,0.5f,1.0f},
				},
				.end{
					.min = {0.1f,0.2f,0.2f,0.2f},
					.max = {0.5f,0.2f,0.2f,0.5f},
				},
			},
		},

		.frequency{
			.interval = 10,
			.isLoop = true,
			//.lifeTime = 120,
		},

		.particleLifeSpan{
			.range{
				.min=60,
				.max=90,
			}
		},

		.textureIndex = TextureManager::GetInstance()->GetTexture(gpuTexture_).GetDescriptorIndex(),

		.createParticleNum = 1 << 10,
		};
		gpuParticleManager_->CreateParticle(emitterForGPU);
	}
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
	//// 0
	//{
	//	Emitter emitterForGPU = {
	//	.min = { -1.0f,-1.0f,-1.0f},
	//	.createParticleNum = 1 << 10,
	//	.max = { 1.0f,1.0f,1.0f},
	//	.isAlive = true,
	//	.position = {player_->GetWorldTransform().translation_},
	//	.time = 0,
	//	.interval = 10,
	//	.isLoop = false,
	//	.textureIndex = TextureManager::GetInstance()->GetTexture(gpuTexture_).GetDescriptorIndex(),
	//	};
	//	gpuParticleManager_->CreateParticle(emitterForGPU);
	//}

	worldTransform_.UpdateMatrix();
	ModelManager::GetInstance()->GetModel(modelHandle_).SetMaterialColor(color_);
}

void GameScene::Draw(CommandContext& commandContext) {
	gpuParticleManager_->Draw(*viewProjection_, commandContext);

	player_->Draw(*viewProjection_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, *viewProjection_, modelHandle_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, *viewProjection_, terrainHandle_, commandContext);
}

void GameScene::Finalize() {}
