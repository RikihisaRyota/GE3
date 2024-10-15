#include "PlayerBullet.h"

#include <numbers>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/Collision/CollisionManager.h"


#include "Src/Game/Boss/Boss.h"

void PlayerBullet::Create(GPUParticleManager* GPUParticleManager, const Vector3& position, const Vector3& velocity, uint32_t time, const BulletEmitter& emitter) {
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Bullet/bullet.gltf");
	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	gpuParticleManager_ = GPUParticleManager;
	velocity_ = velocity;
	time_ = time;
	GPUParticleShaderStructs::NonSharedCopy(emitter_.sharp, emitter.sharp);
	GPUParticleShaderStructs::NonSharedCopy(emitter_.crescent, emitter.crescent);
	GPUParticleShaderStructs::NonSharedCopy(emitter_.field, emitter.field);
	GPUParticleShaderStructs::NonSharedCopy(emitter_.bullet, emitter.bullet);
	GPUParticleShaderStructs::NonSharedCopy(emitter_.bulletShape, emitter.bulletShape);

	isAlive_ = true;

	worldTransform_.Initialize();
	worldTransform_.translate = position;
	worldTransform_.rotate = MakeLookRotation(Normalize(velocity));
	worldTransform_.UpdateMatrix();
#pragma region コライダー
	collider_ = std::make_unique<SphereCollider>();
	collider_->SetName("PlayerBullet");
	auto& mesh = ModelManager::GetInstance()->GetModel(modelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	radius_ = (std::min)(std::fabsf(modelSize.x * worldTransform_.scale.x), (std::min)(std::fabsf(modelSize.y * worldTransform_.scale.y), std::fabsf(modelSize.z * worldTransform_.scale.z)));
	collider_->SetRadius(radius_);
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::PlayerBullet);
	collider_->SetCollisionMask(CollisionAttribute::GameObject | CollisionAttribute::Boss);
	collider_->SetIsActive(true);
#pragma endregion
}

void PlayerBullet::Update() {
	time_--;
	if (time_ <= 0) {
		isAlive_ = false;
		emitter_.bulletShape.isAlive = isAlive_;
	}
	worldTransform_.translate += velocity_;
	UpdateTransform();
	emitter_.bulletShape.emitterArea.position = MakeTranslateMatrix(worldTransform_.matWorld);
	gpuParticleManager_->SetEmitter(emitter_.bulletShape);
}

void PlayerBullet::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(worldTransform_.matWorld, viewProjection, modelHandle_, commandContext);
}

void PlayerBullet::DrawDebug() {
	collider_->DrawCollision({ 0.0f, 1.0f, 0.0f, 1.0f });
}

void PlayerBullet::OnCollision(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "GameObject") {
		isAlive_ = false;
		emitter_.bulletShape.isAlive = isAlive_;
		gpuParticleManager_->SetEmitter(emitter_.bulletShape);
	}
	if (desc.collider->GetName().find("Boss") != std::string::npos) {
		isAlive_ = false;
		Vector3 position = MakeTranslateMatrix(worldTransform_.matWorld);
		emitter_.sharp.emitterArea.position = position;
		emitter_.crescent.emitterArea.position = position;
		emitter_.bullet.emitterArea.position = position;
		emitter_.field.fieldArea.position = position;
		gpuParticleManager_->SetEmitter(emitter_.crescent);
		gpuParticleManager_->SetEmitter(emitter_.sharp);
		gpuParticleManager_->SetEmitter(emitter_.bullet);
		gpuParticleManager_->SetField(emitter_.field);

		emitter_.bulletShape.isAlive = isAlive_;
		gpuParticleManager_->SetEmitter(emitter_.bulletShape);
	}
}

void PlayerBullet::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld));
}
