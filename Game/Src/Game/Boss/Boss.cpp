#include "Boss.h"

#include <numbers>

#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Collision/CollisionAttribute.h"

Boss::Boss() {
	bossModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/boss.gltf");
	animation_.Initialize(bossModelHandle_);

#pragma region コライダー
	collider_ = new OBBCollider();
	collider_->SetName("Boss");
	auto& mesh = ModelManager::GetInstance()->GetModel(bossModelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::Boss);
	collider_->SetCollisionMask(~CollisionAttribute::Boss);
	collider_->SetIsActive(true);
#pragma endregion
	worldTransform_.Initialize();
	animationTransform_.Initialize();
}

void Boss::Initialize() {
	worldTransform_.Reset();
	worldTransform_.translate.z = 5.0f;
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	UpdateTransform();
}

void Boss::Update() {
	colliderColor_ = { 0.0f,0.0f,1.0f,1.0f };
	static const float kCycle = 360.0f;
	animationTime_ += 1.0f;
	animationTime_ = std::fmodf(animationTime_, kCycle);
	animation_.Update(animationTime_ / kCycle);
	UpdateTransform();
}

void Boss::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(animationTransform_, animation_, viewProjection, bossModelHandle_, commandContext);
	//animation_.DrawBox(animationTransform_,viewProjection);
	animation_.DrawLine(animationTransform_);
	collider_->DrawCollision(viewProjection, colliderColor_);
}

void Boss::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	auto& mesh = ModelManager::GetInstance()->GetModel(bossModelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
	animationTransform_.UpdateMatrix();
}

void Boss::OnCollision(const ColliderDesc& desc) {
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	desc;
}
