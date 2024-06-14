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
#include "Engine/LevelDataLoader/LevelDataLoader.h"

GameScene::GameScene() {
	LevelDataLoader::Load("Resources/object.json");

	debugCamera_ = std::make_unique<DebugCamera>();
	//gpuParticleEditor_ = std::make_unique<GPUParticleEditor>();
	gpuParticleManager_ = std::make_unique<GPUParticleManager>();
	player_ = std::make_unique<Player>();
	boss_ = std::make_unique<Boss>();
	followCamera_ = std::make_unique<FollowCamera>();
	skybox_ = std::make_unique<Skybox>();

	for (auto& object : LevelDataLoader::objectData_.gameObject) {
		if (object.transform.parent == -1) {
			gameObject_.emplace_back(std::make_unique<GameObject>(object));
		}
		else {
			gameObject_.emplace_back(std::make_unique<GameObject>(object, &gameObject_.at(object.transform.parent)->GetWorldTransform()));
		}
	}

	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	color_ = { 1.0f,1.0f,1.0f,1.0 };
	soundHandle_ = Audio::GetInstance()->SoundLoad("Resources/Audios/walk.mp3");
	playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);

	worldTransform_.Initialize();

	//gpuParticleEditor_->Initialize();
	gpuParticleManager_->Initialize();

	player_->SetGPUParticleManager(gpuParticleManager_.get());
	boss_->SetGPUParticleManager(gpuParticleManager_.get());
}

GameScene::~GameScene() {}

void GameScene::Initialize() {
	//DrawLine::GetInstance()->SetLine({ -10.0f,1.0f,0.0f }, { 10.0f,1.0f,0.0f }, { 0.0f,1.0f,0.0f,1.0f });
	//Audio::GetInstance()->SoundPlayLoopStart(playHandle_);


	// しゃーなし
	player_->SetViewProjection(viewProjection_);
	player_->Initialize();
	boss_->Initialize();

	skybox_->Initialize();

	followCamera_->SetTarget(&player_->GetWorldTransform());
	followCamera_->SetViewProjection(viewProjection_);
	followCamera_->Initialize();

	auto itData = LevelDataLoader::objectData_.gameObject.begin();
	for (auto& object : gameObject_) {
		if (itData != LevelDataLoader::objectData_.gameObject.end()) {
			object->Initialize(*itData);
			++itData;
		}
		else {
			break;
		}
	}
	//// 0
	//{
	//	GPUParticleShaderStructs::Emitter emitterForGPU = {
	//   .emitterArea{
	//		   .sphere{
	//				.radius = {10.0f},
	//			},
	//			.position{0.0f,0.0f,0.0},
	//			.type = 1,
	//	   },

	//   .scale{
	//	   .range{
	//		   .start{
	//			   .min = {0.1f,0.1f,0.1f},
	//			   .max = {0.2f,0.2f,0.2f},
	//		   },
	//		   .end{
	//			   .min = {0.02f,0.02f,0.02f},
	//			   .max = {0.01f,0.01f,0.01f},
	//		   },
	//	   },
	//   },

	//   .rotate{
	//	   .rotate = 0.0f,
	//   },

	//   .velocity{
	//	   .range{
	//		   .min = {-0.05f,-0.05f,-0.05f},
	//		   .max = {0.05f,0.0f,0.05f},
	//	   }
	//   },

	//   .color{
	//	   .range{
	//		   .start{
	//			   .min = {1.0f,0.0f,0.0f,1.0f},
	//			   .max = {1.0f,0.0f,0.0f,1.0f},
	//		   },
	//		   .end{
	//			   .min = {0.2f,0.0f,0.0f,1.0f},
	//			   .max = {0.2f,0.0f,0.0f,1.0f},
	//		   },
	//	   },
	//   },

	//   .frequency{
	//	   .interval = 1,
	//	   .isLoop = false,
	//	   .lifeTime = 360,
	//   },

	//   .particleLifeSpan{
	//	   .range{
	//		   .min = 10,
	//		   .max = 30,
	//	   }
	//   },

	//   .textureIndex = TextureManager::GetInstance()->GetTexture(gpuTexture_).GetDescriptorIndex(),

	//   .createParticleNum = 1 << 10,
	//	};

	//	gpuParticleManager_->CreateParticle(emitterForGPU);
	//}

	//// 0
	//{
	//	GPUParticleShaderStructs::Emitter emitterForGPU = {
	//   .emitterArea{
	//			.aabb{
	//				.area{
	//					.min = {-10.0f,-10.0f,-20.0f},
	//					.max = {10.0f,10.0f,20.0f},
	//					},
	//			},
	//			.position = {0.0f,0.0f,0.0f},
	//			.type = 0,

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
	//	   .rotate = 0.3f,
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
	//	   .interval = 2,
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

void GameScene::Update(CommandContext& commandContext) {
	DrawLine::GetInstance()->SetLine({ -10.0f,0.0f,0.0f }, { 10.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f,1.0f });
	
	skybox_->Update();

	debugCamera_->Update(viewProjection_);

	if (!debugCamera_->GetIsDebugCamera()) {
		followCamera_->Update();
	}
	player_->Update(commandContext);
	boss_->Update(commandContext);
	for (auto& object : gameObject_) {
		object->Update();
	}

	worldTransform_.UpdateMatrix();
	gpuParticleManager_->Update(RenderManager::GetInstance()->GetCommandContext());
	//gpuParticleEditor_->Update(RenderManager::GetInstance()->GetCommandContext());

	CollisionManager::GetInstance()->Collision();

#ifdef _DEBUG
	player_->DrawImGui();
	boss_->DrawImGui();
	for ( auto & object : gameObject_) {
		object->DrawImGui();
	}
#endif // _DEBUG
}

void GameScene::Draw(CommandContext& commandContext) {

	player_->Draw(*viewProjection_, commandContext);
	boss_->Draw(*viewProjection_, commandContext);
	for (auto& object : gameObject_) {
		object->Draw(*viewProjection_, commandContext);
	}

	//gpuParticleEditor_->Draw(*viewProjection_, commandContext);

	skybox_->Draw(commandContext, *viewProjection_);
	gpuParticleManager_->Draw(*viewProjection_, commandContext);
}

void GameScene::Finalize() {}
