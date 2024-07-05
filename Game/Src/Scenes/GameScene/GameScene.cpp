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
#include "Engine/Input/Input.h"
#include "Engine/Json/JsonUtils.h"

GameScene::GameScene() {
	CollisionManager::GetInstance()->ClearCollider();
	LevelDataLoader::Load("Resources/object.json");

	debugCamera_ = std::make_unique<DebugCamera>();
	//gpuParticleEditor_ = std::make_unique<GPUParticleEditor>();
	gpuParticleManager_ = std::make_unique<GPUParticleManager>();
	// 当たり判定の順番
	player_ = std::make_unique<Player>();
	for (auto& object : LevelDataLoader::objectData_.gameObject) {
		if (object.transform.parent == -1) {
			gameObject_.emplace_back(std::make_unique<GameObject>(object));
		}
		else {
			gameObject_.emplace_back(std::make_unique<GameObject>(object, &gameObject_.at(object.transform.parent)->GetWorldTransform()));
		}
	}
	boss_ = std::make_unique<Boss>();
	followCamera_ = std::make_unique<FollowCamera>();
	skybox_ = std::make_unique<Skybox>();


	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");

	soundHandle_ = Audio::GetInstance()->SoundLoad("Resources/Audios/walk.mp3");
	playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);

	//gpuParticleEditor_->Initialize();
	gpuParticleManager_->Initialize();

	player_->SetBoss(boss_.get());
	player_->SetGPUParticleManager(gpuParticleManager_.get());
	boss_->SetGPUParticleManager(gpuParticleManager_.get());

	GPUParticleShaderStructs::Load("test", test_);

}

GameScene::~GameScene() {
	CollisionManager::GetInstance();
}

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
	//	test_.emitterArea.aabb.area.min = { -10.0f,-10.0f,-20.0f };
	//	test_.emitterArea.aabb.area.max = { 10.0f,10.0f,20.0f };
	//	test_.emitterArea.position = { 0.0f,0.0f,0.0f };
	//	test_.emitterArea.type = GPUParticleShaderStructs::Type::kAABB;

	//	test_.scale.range.start.min = { 0.1f,0.1f,0.1f };
	//	test_.scale.range.start.max = { 0.3f,0.3f,0.3f };
	//	test_.scale.range.end.max = { 0.1f,0.1f,0.1f };
	//	test_.scale.range.end.max = { 0.1f,0.1f,0.1f };

	//	test_.rotate.rotate = 0.3f;

	//	test_.velocity.range.min = { 0.0f,0.0f,0.1f };
	//	test_.velocity.range.max = { 0.0f,0.0f,0.5f };

	//	test_.color.range.start.min = { 0.5f,0.5f,0.5f,1.0f };
	//	test_.color.range.start.max = { 0.5f,0.5f,0.5f,1.0f };
	//	test_.color.range.end.min = { 0.5f,0.5f,0.5f,1.0f };
	//	test_.color.range.end.max = { 0.5f,0.5f,0.5f,1.0f };

	//	test_.frequency.interval = 5;
	//	test_.frequency.isLoop = true;
	//	test_.frequency.emitterLife = 120;

	//	test_.particleLifeSpan.range.min = 1;
	//	test_.particleLifeSpan.range.min = 90;

	//	test_.textureIndex = TextureManager::GetInstance()->GetTexture(gpuTexture_).GetDescriptorIndex(),

	//		test_.createParticleNum = 333;
		//gpuParticleManager_->SetEmitter(test_);
	//}
}

void GameScene::Update(CommandContext& commandContext) {

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


	//gpuParticleManager_->SetEmitter(test_);
	gpuParticleManager_->Update(*viewProjection_, RenderManager::GetInstance()->GetCommandContext());
	//gpuParticleEditor_->Update(RenderManager::GetInstance()->GetCommandContext());

	CollisionManager::GetInstance()->Collision();
#ifdef _DEBUG
	gpuParticleManager_->DrawImGui();
	skybox_->DrawImGui();
	followCamera_->DrawImGui();
	player_->DrawImGui();
	boss_->DrawImGui();
	for (auto& object : gameObject_) {
		object->DrawImGui();
	}
	GPUParticleShaderStructs::Debug("test", test_);
	GPUParticleShaderStructs::Update();
	if (Input::GetInstance()->PushKey(DIK_R)) {
		skybox_->Initialize();
		followCamera_->Initialize();
		player_->Initialize();
		boss_->Initialize();
		for (size_t i = 0; auto & object : LevelDataLoader::objectData_.gameObject) {
			gameObject_.at(i)->Initialize(object);
			i++;
		}
	}
#endif // _DEBUG
}

void GameScene::Draw(CommandContext& commandContext) {
	player_->Draw(*viewProjection_, commandContext);
	boss_->Draw(*viewProjection_, commandContext);
	for (auto& object : gameObject_) {
		object->Draw(*viewProjection_, commandContext);
	}


	gpuParticleManager_->Draw(*viewProjection_, commandContext);
	//skybox_->Draw(commandContext, *viewProjection_);
	player_->DrawSprite(commandContext);
#ifdef _DEBUG
	static bool playerDebug = false;
	static bool bossDebug = false;
	ImGui::Checkbox("PlayerDebug", &playerDebug);
	if (playerDebug) {
		player_->DrawDebug(*viewProjection_);
	}
	ImGui::Checkbox("BossDebug", &bossDebug);
	if (bossDebug) {
		boss_->DrawDebug(*viewProjection_);
	}
	for (auto& object : gameObject_) {
		object->DrawDebug(*viewProjection_);
	}
#endif // _DEBUG

}

void GameScene::Finalize() {}
