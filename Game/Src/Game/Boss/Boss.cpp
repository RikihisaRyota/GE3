#include "Boss.h"

#include "Engine/Model/ModelManager.h"

Boss::Boss() {
	bossModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/boss.gltf");
	animation_.Initialize(bossModelHandle_);
}

void Boss::Initialize() {

	worldTransform_.Initialize();
	animationTransform_.Initialize();
	animationTransform_.parent_ = &worldTransform_;
	worldTransform_.translate = { 0.0f,0.0f,10.0f };
}

void Boss::Update() {
	static const float kCycle = 360.0f;
	animationTime_ += 1.0f;
	animationTime_ = std::fmodf(animationTime_, kCycle);
	animation_.Update(animationTime_ / kCycle);
	UpdateTransform();
}

void Boss::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(animationTransform_, animation_, viewProjection, bossModelHandle_, commandContext);
	//ModelManager::GetInstance()->Draw(animationTransform_,  viewProjection, bossModelHandle_, commandContext);
	animation_.Draw(animationTransform_);
}

void Boss::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	animationTransform_.UpdateMatrix();
}
