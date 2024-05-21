#include "GameScene.h"

#include "Engine/Audio/Audio.h"
#include "Engine/DrawLine/DrawLine.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Graphics/RenderManager.h"
#include "Engine//Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Sprite/SpriteManager.h"
#include "Engine/Collision/CollisionManager.h"

GameScene::GameScene() {
	debugCamera_ = std::make_unique<DebugCamera>();
	//gpuParticleEditor_ = std::make_unique<GPUParticleEditor>();
	gpuParticleManager_ = std::make_unique<GPUParticleManager>();
	player_ = std::make_unique<Player>();
	boss_ = std::make_unique<Boss>();
	followCamera_ = std::make_unique<FollowCamera>();

	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Ball/Ball.obj");
	terrainHandle_ = ModelManager::GetInstance()->Load("Resources/Models/terrain/terrain.obj");

	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	color_ = { 1.0f,1.0f,1.0f,1.0 };
	soundHandle_ = Audio::GetInstance()->SoundLoad("Resources/Audios/walk.mp3");
	playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);

	animationModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Player/player.gltf");
	animation_.Initialize(animationModelHandle_);

	animationWorldTransform_.Initialize();;
	animationTime_ = 0.0f;
	worldTransform_.Initialize();

	//gpuParticleEditor_->Initialize();
	//gpuParticleManager_->Initialize();
}

GameScene::~GameScene() {}

void GameScene::Initialize() {
	//DrawLine::GetInstance()->SetLine({ -10.0f,1.0f,0.0f }, { 10.0f,1.0f,0.0f }, { 0.0f,1.0f,0.0f,1.0f });
	//Audio::GetInstance()->SoundPlayLoopStart(playHandle_);

	player_->SetViewProjection(viewProjection_);
	player_->SetGPUParticleManager(gpuParticleManager_.get());
	player_->Initialize();
	boss_->Initialize();

	followCamera_->SetTarget(&player_->GetWorldTransform());
	followCamera_->SetViewProjection(viewProjection_);
	followCamera_->Initialize();
	// 0
	{
		GPUParticleShaderStructs::Emitter emitterForGPU = {
	   .emitterArea{
			   .area{
				   .min = {-30.0f,-30.0f,-30.0f},
				   .max = {30.0f,30.0f,30.0f},
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
				   .min = {0.02f,0.02f,0.02f},
				   .max = {0.01f,0.01f,0.01f},
			   },
		   },
	   },

	   .rotate{
		   .rotate = {0.0f,0.0f,0.0f},
	   },

	   .velocity{
		   .range{
			   .min = {-0.05f,-0.05f,-0.05f},
			   .max = {0.05f,0.0f,0.05f},
		   }
	   },

	   .color{
		   .range{
			   .start{
				   .min = {1.0f,1.0f,1.0f,1.0f},
				   .max = {1.0f,1.0f,1.0f,1.0f},
			   },
			   .end{
				   .min = {0.5f,0.5f,0.5f,1.0f},
				   .max = {0.5f,0.5f,0.5f,1.0f},
			   },
		   },
	   },

	   .frequency{
		   .interval = 1,
		   .isLoop = true,
		   //.lifeTime = 120,
	   },

	   .particleLifeSpan{
		   .range{
			   .min = 10,
			   .max = 30,
		   }
	   },

	   .textureIndex = TextureManager::GetInstance()->GetTexture(gpuTexture_).GetDescriptorIndex(),

	   .createParticleNum = 1 << 15,
		};

		//gpuParticleManager_->CreateParticle(emitterForGPU);
	}

	// 0
	//{
	//	GPUParticleShaderStructs::Emitter emitterForGPU = {
	//   .emitterArea{
	//		   .area{
	//			   .min = {-10.0f,-10.0f,-20.0f},
	//			   .max = {10.0f,10.0f,20.0f},
	//		   },
	//		   .position = {0.0f,0.0f,0.0f},
	//	   },

	//   .scale{
	//	   .range{
	//		   .start{
	//			   .min = {0.01f,0.01f,0.01f},
	//			   .max = {0.05f,0.05f,0.05f},
	//		   },
	//		   .end{
	//			   .min = {0.1f,0.1f,0.1f},
	//			   .max = {0.1f,0.1f,0.1f},
	//		   },
	//	   },
	//   },

	//   .rotate{
	//	   .rotate = {0.0f,0.0f,0.3f},
	//   },

	//   .velocity{
	//	   .range{
	//		   .min = {0.0f,0.0f,0.1f},
	//		   .max = {0.0f,0.0f,0.5f},
	//	   }
	//   },

	//   .color{
	//	   .range{
	//		   .start{
	//			   .min = {0.5f,0.5f,0.5f,1.0f},
	//			   .max = {0.5f,0.5f,0.5f,1.0f},
	//		   },
	//		   .end{
	//			   .min = {0.01f,0.01f,0.01f,0.01f},
	//			   .max = {0.01f,0.01f,0.01f,0.01f},
	//		   },
	//	   },
	//   },

	//   .frequency{
	//	   .interval = 3,
	//	   .isLoop = true,
	//	   //.lifeTime = 120,
	//   },

	//   .particleLifeSpan{
	//	   .range{
	//		   .min = 1,
	//		   .max = 90,
	//	   }
	//   },

	//   .textureIndex = TextureManager::GetInstance()->GetTexture(gpuTexture_).GetDescriptorIndex(),

	//   .createParticleNum = 1 << 10,
	//	};
	//	gpuParticleManager_->CreateParticle(emitterForGPU);
	//}
}

void GameScene::Update() {
	DrawLine::GetInstance()->SetLine({ -10.0f,0.0f,0.0f }, { 10.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f });
	//DrawLine::GetInstance()->SetLine({ -10.0f,1.0f,0.0f }, { 10.0f,1.0f,0.0f }, { 0.0f,1.0f,0.0f,1.0f });
	//DrawLine::GetInstance()->SetLine({ -10.0f,1.0f,0.0f }, { 10.0f,1.0f,0.0f }, { 0.0f,1.0f,0.0f,1.0f });
	//DrawLine::GetInstance()->SetLine({ -10.0f,1.0f,0.0f }, { 10.0f,1.0f,0.0f }, { 0.0f,1.0f,0.0f,1.0f });

	debugCamera_->Update(viewProjection_);

	static const float kCycle = 60.0f;
	animationTime_ += 1.0f;
	animationTime_ = std::fmodf(animationTime_, kCycle);
	//animation_.Update(animationTime_/ kCycle);

	animationWorldTransform_.TransferMatrix();

	if (!debugCamera_->GetIsDebugCamera()) {
		followCamera_->Update();
	}
	player_->Update();
	boss_->Update();

	worldTransform_.UpdateMatrix();
	ModelManager::GetInstance()->GetModel(modelHandle_).SetMaterialColor(color_);

	//gpuParticleManager_->Update(RenderManager::GetInstance()->GetCommandContext());
	//gpuParticleEditor_->Update(RenderManager::GetInstance()->GetCommandContext());

	CollisionManager::GetInstance()->Collision();
}

void GameScene::Draw(CommandContext& commandContext) {
	
	player_->Draw(*viewProjection_, commandContext);
	boss_->Draw(*viewProjection_, commandContext);

	ModelManager::GetInstance()->Draw(animationWorldTransform_, animation_, *viewProjection_, animationModelHandle_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, *viewProjection_, modelHandle_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, *viewProjection_, terrainHandle_, commandContext);

	//gpuParticleManager_->Draw(*viewProjection_, commandContext);
	//gpuParticleEditor_->Draw(*viewProjection_, commandContext);
}

void GameScene::Finalize() {}
