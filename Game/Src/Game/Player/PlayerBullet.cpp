#include "PlayerBullet.h"

#include <numbers>

#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/Graphics/CommandContext.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/Collision/CollisionManager.h"

PlayerBullet::~PlayerBullet() {
	CollisionManager::GetInstance()->DeleteCollider(collider_);
}
void PlayerBullet::Create(GPUParticleManager* GPUParticleManager, const Vector3& position, const Vector3& velocity, uint32_t time) {
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Bullet/bullet.gltf");
	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	gpuParticleManager_ = GPUParticleManager;
	velocity_ = velocity;
	time_ = time;
	isAlive_ = true;

	worldTransform_.Initialize();
	worldTransform_.translate = position;
	worldTransform_.rotate = MakeLookRotation(Normalize(velocity));
	worldTransform_.UpdateMatrix();
#pragma region コライダー
	collider_ = new OBBCollider();
	collider_->SetName("PlayerBullet");
	auto& mesh = ModelManager::GetInstance()->GetModel(modelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::PlayerBullet);
	collider_->SetCollisionMask(CollisionAttribute::GameObject| CollisionAttribute::Boss);
	collider_->SetIsActive(true);
#pragma endregion
}

void PlayerBullet::Update() {
	time_--;
	if (time_ <= 0) {
		isAlive_ = false;
	}
	worldTransform_.translate += velocity_;
	UpdateTransform();
}

void PlayerBullet::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(worldTransform_, viewProjection, modelHandle_, commandContext);
}

void PlayerBullet::DrawDebug(const ViewProjection& viewProjection) {
	collider_->DrawCollision(viewProjection,Vector4(0.0f,1.0f,0.0f,1.0f));
}

void PlayerBullet::OnCollision(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "Boss" ||
		desc.collider->GetName() == "GameObject") {
		isAlive_ = false;
	}
}

void PlayerBullet::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	auto& mesh = ModelManager::GetInstance()->GetModel(modelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
}
