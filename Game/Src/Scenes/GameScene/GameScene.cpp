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
	TextureManager::GetInstance()->Load("Resources/Images/GPUParticle_1.png");
	TextureManager::GetInstance()->Load("Resources/Images/smoke.png");
	TextureManager::GetInstance()->Load("Resources/Images/crescent.png");
	TextureManager::GetInstance()->Load("Resources/Images/noise.png");
	TextureManager::GetInstance()->Load("Resources/Images/te.png");


	//soundHandle_ = Audio::GetInstance()->SoundLoad("Resources/Audios/walk.mp3");
	//playHandle_ = Audio::GetInstance()->SoundPlayLoopStart(soundHandle_);

	//gpuParticleEditor_->Initialize();
	gpuParticleManager_->Initialize();

	player_->SetBoss(boss_.get());
	boss_->SetPlayer(player_.get());
	player_->SetGPUParticleManager(gpuParticleManager_.get());
	boss_->SetGPUParticleManager(gpuParticleManager_.get());

	GPUParticleShaderStructs::Load("postEmitter", postEmitter_);
	GPUParticleShaderStructs::Load("groundEmitter", groundEmitter_);

#ifdef _DEBUG
	GPUParticleShaderStructs::Load("test", testEmitter_);
	GPUParticleShaderStructs::Load("test1", test1Emitter_);
	GPUParticleShaderStructs::Load("test2", test2Emitter_);
	GPUParticleShaderStructs::Load("test", testField_);
	//GPUParticleShaderStructs::Load("test", testVertexEmitter_);

	//testModel_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/cannon.gltf");
	//testWorldTransform_.Initialize();
#endif // _DEBUG
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
}

void GameScene::Update(CommandContext& commandContext) {

	//gpuParticleManager_->SetVertexEmitter(testModel_, testVertexEmitter_);

	skybox_->Update();

	debugCamera_->Update(viewProjection_);

	if (!debugCamera_->GetIsDebugCamera()) {
		followCamera_->Update();
	}
	player_->Update(commandContext);
	boss_->Update(commandContext);
	for (auto& object : gameObject_) {
		object->Update();
		if (object->GetObjectName() == "Post") {
			GPUParticleShaderStructs::EmitterForCPU emitter{};
			GPUParticleShaderStructs::NonSharedCopy(emitter, postEmitter_);
			emitter.emitterArea.capsule.segment.origin = MakeTranslateMatrix(object->GetWorldTransform().matWorld) + postEmitter_.emitterArea.capsule.segment.origin;
			emitter.emitterArea.capsule.segment.diff = MakeTranslateMatrix(object->GetWorldTransform().matWorld) + postEmitter_.emitterArea.capsule.segment.diff;
			gpuParticleManager_->SetEmitter(emitter);
		}
		if (object->GetObjectName() == "Ground") {
			postEmitter_.emitterArea.position = MakeTranslateMatrix(object->GetWorldTransform().matWorld);
			//gpuParticleManager_->SetEmitter(groundEmitter_);
		}
	}

	gpuParticleManager_->Update(*viewProjection_, commandContext);

	CollisionManager::GetInstance()->Collision();
#ifdef _DEBUG
	RenderManager::GetInstance()->GetHSVFilter().Debug();
	RenderManager::GetInstance()->GetBloom().Debug();
	ImGui::DragFloat3("Scale", &testWorldTransform_.scale.x, 0.1f);
	ImGui::DragFloat3("Translate", &testWorldTransform_.translate.x, 0.1f);
	testWorldTransform_.UpdateMatrix();
	if (ImGui::Button("GameObjectLoad")) {
		LevelDataLoader::Load("Resources/object.json");
		for (auto& object : gameObject_) {
			CollisionManager::GetInstance()->DeleteCollider(object->GetCollider());
		}
		gameObject_.clear();
		for (auto& object : LevelDataLoader::objectData_.gameObject) {
			if (object.transform.parent == -1) {
				gameObject_.emplace_back(std::make_unique<GameObject>(object));
			}
			else {
				gameObject_.emplace_back(std::make_unique<GameObject>(object, &gameObject_.at(object.transform.parent)->GetWorldTransform()));
			}
		}
	}
	gpuParticleManager_->DrawImGui();
	skybox_->DrawImGui();
	followCamera_->DrawImGui();
	player_->DrawImGui();
	boss_->DrawImGui();
	for (auto& object : gameObject_) {
		object->DrawImGui();
	}

	gpuParticleManager_->SetField(testField_);
	gpuParticleManager_->SetEmitter(testEmitter_);
	gpuParticleManager_->SetEmitter(test1Emitter_);
	gpuParticleManager_->SetEmitter(test2Emitter_);
	GPUParticleShaderStructs::Debug("test", testField_);
	GPUParticleShaderStructs::Debug("test", testEmitter_);
	GPUParticleShaderStructs::Debug("postEmitter", postEmitter_);
	GPUParticleShaderStructs::Debug("groundEmitter", groundEmitter_);
	//GPUParticleShaderStructs::Debug("test", testVertexEmitter_);
	GPUParticleShaderStructs::Debug("test1", test1Emitter_);
	GPUParticleShaderStructs::Debug("test2", test2Emitter_);
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
	//ModelManager::GetInstance()->Draw(testWorldTransform_.matWorld, *viewProjection_, testModel_, commandContext);
	player_->Draw(*viewProjection_, commandContext);
	boss_->Draw(*viewProjection_, commandContext);
	for (auto& object : gameObject_) {
	/*	if (object->GetObjectName() != "Post") {
			object->Draw(*viewProjection_, commandContext);
		}*/
		if (object->GetObjectName() == "Ground") {
			object->Draw(*viewProjection_, commandContext);
		}
	}


	gpuParticleManager_->Draw(*viewProjection_, commandContext);
	//skybox_->Draw(commandContext, *viewProjection_);
	player_->DrawSprite(commandContext);
#ifdef _DEBUG
	static bool playerDebug = false;
	static bool bossDebug = false;
	ImGui::Checkbox("PlayerDebug", &playerDebug);
	if (playerDebug) {
		player_->DrawDebug();
	}
	ImGui::Checkbox("BossDebug", &bossDebug);
	if (bossDebug) {
		boss_->DrawDebug();
	}
	//for (auto& object : gameObject_) {
	//	object->DrawDebug();
	//}
	GPUParticleShaderStructs::DebugDraw(testEmitter_);
	GPUParticleShaderStructs::DebugDraw(test1Emitter_);
	GPUParticleShaderStructs::DebugDraw(test2Emitter_);
	GPUParticleShaderStructs::DebugDraw(testField_);
	GPUParticleShaderStructs::DebugDraw(groundEmitter_);
#endif // _DEBUG

}

void GameScene::Finalize() {}
