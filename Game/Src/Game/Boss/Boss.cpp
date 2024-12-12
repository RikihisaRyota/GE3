#include "Boss.h"

#include <numbers>

#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/GPUParticleManager/GPUParticleManager.h"

#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Texture/TextureManager.h"

#include "Engine/Json/JsonUtils.h"

Boss::Boss() {
	// モデル読み込み
	ModelManager::GetInstance()->Load("Resources/Models/Boss/baggy.gltf");
	ModelManager::GetInstance()->Load("Resources/Models/Boss/train.gltf");
	ModelManager::GetInstance()->Load("Resources/Models/Boss/rail.gltf");
	ModelManager::GetInstance()->Load("Resources/Models/Boss/hand.gltf");
	ModelManager::GetInstance()->Load("Resources/Models/Boss/cannon.gltf");
	bossModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/boss.gltf");
	//animation_.Initialize("Resources/Animation/Boss/animation.gltf", bossModelHandle_);
	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	bossStateManager_ = std::make_unique<BossStateManager>();
	bossStateManager_->SetBoss(this);
	worldTransform_.Initialize();
	animationTransform_.Initialize();
	collisionTransform_.Initialize();

	bossHP_ = std::make_unique<BossHP>();

	collider_ = std::make_unique<SphereCollider>();

	// Json読み込み
	float radius = 0.0f;
	JSON_OPEN("Resources/Data/Boss/bossCollider.json");
	JSON_OBJECT("Collider");
	JSON_LOAD_BY_NAME("radius", radius);
	JSON_CLOSE();
	collider_->SetRadius(radius);
	collider_->SetName("Boss");
	//collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::Boss);
	collider_->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
	collider_->SetIsActive(true);

	JSON_OPEN("Resources/Data/Boss/boss.json");
	JSON_OBJECT("bossProperties");
	JSON_LOAD(offset_);
	JSON_LOAD(collisionWorldTransformOffset_);
	JSON_LOAD(animationWorldTransformOffset_);
	JSON_ROOT();
	JSON_CLOSE();

	GPUParticleShaderStructs::Load("boss", meshEmitterDesc_);
	GPUParticleShaderStructs::Load("boss", vertexEmitterDesc_);
}

void Boss::Initialize() {
	bossStateManager_->Initialize();
	bossHP_->Initialize();
	worldTransform_.Reset();
	worldTransform_.translate = offset_;
	worldTransform_.rotate = MakeRotateYAngleQuaternion(DegToRad(180.0f));
	collisionTransform_.Reset();
	collisionTransform_.parent_ = &worldTransform_;
	collisionTransform_.translate = collisionWorldTransformOffset_;
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	animationTransform_.translate = animationWorldTransformOffset_;
	UpdateTransform();
	UpdateCollider();
}

void Boss::Update(CommandContext& commandContext) {
	bossStateManager_->Update(commandContext);
	UpdateTransform();
	UpdateGPUParticle(commandContext);
	UpdateCollider();
}

void Boss::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	//ModelManager::GetInstance()->Draw(animationTransform_.matWorld, viewProjection, bossModelHandle_, commandContext);
}

void Boss::DrawImGui() {
#ifdef _DEBUG
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Boss")) {
		ImGui::Checkbox("isAliveEmitter", reinterpret_cast<bool*>(&vertexEmitterDesc_.isAlive));
		ImGui::DragFloat3("translate", &worldTransform_.translate.x, 0.1f);
		ImGui::DragFloat3("collisionTranslate", &collisionTransform_.translate.x, 0.1f);
		ImGui::DragFloat3("animationTranslate", &animationTransform_.translate.x, 0.1f);
		ImGui::Text("matWorld: x:%f,y:%f,z:%f", MakeTranslateMatrix(worldTransform_.matWorld).x, MakeTranslateMatrix(worldTransform_.matWorld).y, MakeTranslateMatrix(worldTransform_.matWorld).z, 0.1f);
		if (ImGui::TreeNode("Properties")) {
			ImGui::DragFloat3("Offset", &offset_.x, 0.1f);
			ImGui::DragFloat3("CollisionWorldTransformOffset", &collisionWorldTransformOffset_.x, 0.1f);
			ImGui::DragFloat3("AnimationWorldTransformOffset", &animationWorldTransformOffset_.x, 0.1f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/boss.json");
				JSON_OBJECT("bossProperties");
				JSON_SAVE(offset_);
				JSON_SAVE(collisionWorldTransformOffset_);
				JSON_SAVE(animationWorldTransformOffset_);
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Collision")) {
			auto& collider = collider_->GetSphere();
			ImGui::DragFloat("Radius", &collider.radius, 0.1f, 0.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/bossCollider.json");
				JSON_OBJECT("Collider");
				JSON_SAVE_BY_NAME("radius", collider.radius);
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	bossStateManager_->DrawImGui();
	bossHP_->DrawImGui();
	GPUParticleShaderStructs::Debug("boss", meshEmitterDesc_);
	GPUParticleShaderStructs::Debug("boss", vertexEmitterDesc_);
	GPUParticleShaderStructs::Debug("boss", transformEmitter_);
#endif // _DEBUG
}

void Boss::DrawDebug() {
	collider_->DrawCollision({ 0.0f,1.0f,0.0f,1.0f });
	animation_.DrawLine(animationTransform_);
}

Vector3 Boss::GetWorldTranslate() const {
	return MakeTranslateMatrix(worldTransform_.matWorld);
}

void Boss::UpdateCollider() {
	if (bossStateManager_->GetCurrentState() == BossStateManager::State::kRoot &&
		!bossStateManager_->GetInTransition()) {
		collider_->SetIsActive(true);
	}
	else {
		collider_->SetIsActive(false);
	}
	collider_->SetCenter(MakeTranslateMatrix(collisionTransform_.matWorld));
}

void Boss::UpdateGPUParticle(CommandContext& commandContext) {
	//gpuParticleManager_->CreateEdgeParticle(bossModelHandle_, animation_, worldTransform_.matWorld, meshEmitterDesc_, commandContext);
	//gpuParticleManager_->CreateMeshParticle(bossModelHandle_, animation_, worldTransform_.matWorld, meshEmitterDesc_, commandContext);
	if (bossStateManager_->GetCurrentState() == BossStateManager::State::kRoot&&
		!bossStateManager_->GetInTransition()) {
		vertexEmitterDesc_.isAlive = true;
		meshEmitterDesc_.isAlive = true;
		//gpuParticleManager_->SetVertexEmitter(bossModelHandle_,vertexEmitterDesc_, worldTransform_.matWorld);
		gpuParticleManager_->SetMeshEmitter(bossModelHandle_,meshEmitterDesc_, worldTransform_.matWorld);
	}
	else {
		vertexEmitterDesc_.isAlive = false;
		meshEmitterDesc_.isAlive = false;
		//gpuParticleManager_->SetVertexEmitter(bossModelHandle_,vertexEmitterDesc_, worldTransform_.matWorld);
		gpuParticleManager_->SetMeshEmitter(bossModelHandle_,meshEmitterDesc_, worldTransform_.matWorld);
	}
	//gpuParticleManager_->CreateTransformModelParticle(bossModelHandle_, worldTransform_.matWorld, testModelHandle_, worldTransform_.matWorld, transformEmitter_, commandContext);
}

void Boss::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	animationTransform_.UpdateMatrix();
	collisionTransform_.UpdateMatrix();
}

void Boss::OnCollisionBody(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "PlayerBullet") {
		bossHP_->HitDamage(1);
	}
}

void Boss::OnCollisionAttack(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "PlayerBullet") {
		bossHP_->HitDamage(1);
	}
}
