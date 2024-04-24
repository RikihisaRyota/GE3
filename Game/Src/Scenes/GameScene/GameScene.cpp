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
	//gpuParticleEditor_ = std::make_unique<GPUParticleEditor>();
	gpuParticleManager_ = std::make_unique<GPUParticleManager>();
	player_ = std::make_unique<Player>();
	followCamera_ = std::make_unique<FollowCamera>();

	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Ball/Ball.obj");
	terrainHandle_ = ModelManager::GetInstance()->Load("Resources/Models/terrain/terrain.obj");

	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	color_ = { 1.0f,1.0f,1.0f,1.0 };
	soundHandle_ = Audio::GetInstance()->SoundLoad("Resources/Audios/walk.mp3");
	playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);

	animationModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Walk/walk.gltf");
	animation_.LoadAnimationFile("Resources/Models/Walk/walk.gltf");
	skeleton_.CreateSkeleton(ModelManager::GetInstance()->GetModel(animationModelHandle_).GetMeshData().at(0)->rootNode);
	skinCluster_.CreateSkinCluster(skeleton_, animationModelHandle_);

	animationWorldTransform_.Initialize();;
	animationTime_ = 0.0f;
	worldTransform_.Initialize();

	//gpuParticleEditor_->Initialize();
	gpuParticleManager_->Initialize();
}

GameScene::~GameScene() {}

void GameScene::Initialize() {

	//Audio::GetInstance()->SoundPlayLoopStart(playHandle_);

	player_->SetViewProjection(viewProjection_);
	player_->SetGPUParticleManager(gpuParticleManager_.get());
	player_->Initialize();

	followCamera_->SetTarget(&player_->GetWorldTransform());
	followCamera_->SetViewProjection(viewProjection_);
	followCamera_->Initialize();
	// 0
	{
		GPUParticleShaderStructs::Emitter emitterForGPU = {
	   .emitterArea{
			   .area{
				   .min = {-1.0f,-5.0f,-5.0f},
				   .max = {1.0f,5.0f,5.0f},
			   },
			   .position = {0.0f,0.0f,0.0f},
		   },

	   .scale{
		   .range{
			   .start{
				   .min = {0.05f,0.05f,0.05f},
				   .max = {0.08f,0.08f,0.08f},
			   },
			   .end{
				   .min = {0.01f,0.01f,0.01f},
				   .max = {0.03f,0.03f,0.03f},
			   },
		   },
	   },

	   .rotate{
		   .rotate = {0.0f,0.0f,0.0f},
	   },

	   .velocity{
		   .range{
			   .min = {0.0f,0.1f,0.0f},
			   .max = {0.0f,0.2f,0.0f},
		   }
	   },

	   .color{
		   .range{
			   .start{
				   .min = {0.5f,0.0f,0.0f,1.0f},
				   .max = {0.8f,0.0f,0.0f,1.0f},
			   },
			   .end{
				   .min = {0.0f,0.0f,0.5f,1.0f},
				   .max = {0.0f,0.0f,0.8f,1.0f},
			   },
		   },
	   },

	   .frequency{
		   .interval = 5,
		   .isLoop = true,
		   //.lifeTime = 120,
	   },

	   .particleLifeSpan{
		   .range{
			   .min = 15,
			   .max = 30,
		   }
	   },

	   .textureIndex = TextureManager::GetInstance()->GetTexture(ModelManager::GetInstance()->GetModel(modelHandle_).GetTextureHandle()).GetDescriptorIndex(),

	   .createParticleNum = 1 << 10,
		};

		gpuParticleManager_->CreateParticle(emitterForGPU);
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

#ifdef ENABLE_IMGUI
	ImGui::Begin("Sphere");
	ImGui::DragFloat3("scale", &worldTransform_.scale.x, 0.1f, 0.0f);
	ImGui::End();
#endif // ENABLE_IMGUI

	debugCamera_->Update(viewProjection_);

	static const float kCycle = 60.0f;
	animationTime_ += 1.0f;
	animationTime_ = std::fmodf(animationTime_, kCycle);
	ApplyAnimation(skeleton_, animation_, animationTime_ / kCycle);
	skeleton_.Update();
	skinCluster_.Update(skeleton_);

	animationWorldTransform_.TransferMatrix();

	if (!debugCamera_->GetIsDebugCamera()) {
		followCamera_->Update();
	}
	player_->Update();

	worldTransform_.UpdateMatrix();
	ModelManager::GetInstance()->GetModel(modelHandle_).SetMaterialColor(color_);

	gpuParticleManager_->Update(RenderManager::GetInstance()->GetCommandContext());
	//gpuParticleEditor_->Update(RenderManager::GetInstance()->GetCommandContext());
}

void GameScene::Draw(CommandContext& commandContext) {
	
	player_->Draw(*viewProjection_, commandContext);

	ModelManager::GetInstance()->Draw(animationWorldTransform_, skinCluster_, *viewProjection_, animationModelHandle_, commandContext);

	//ModelManager::GetInstance()->Draw(worldTransform_, *viewProjection_, modelHandle_, commandContext);

	ModelManager::GetInstance()->Draw(worldTransform_, *viewProjection_, terrainHandle_, commandContext);

	gpuParticleManager_->Draw(*viewProjection_, commandContext);
	//gpuParticleEditor_->Draw(*viewProjection_, commandContext);
}

void GameScene::Finalize() {}
