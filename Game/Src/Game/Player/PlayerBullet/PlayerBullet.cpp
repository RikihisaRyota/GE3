#include "PlayerBullet.h"

#include <numbers>

#include "Engine/Graphics/CommandContext.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Texture/TextureManager.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/Collision/CollisionManager.h"


#include "Src/Game/Boss/Boss.h"

void PlayerBullet::Create(GPUParticleManager* GPUParticleManager, const BulletDesc& desc) {
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Bullet/bullet.gltf");
	gpuTexture_ = TextureManager::GetInstance()->Load("Resources/Images/GPUParticle.png");
	gpuParticleManager_ = GPUParticleManager;
	desc_ = desc;
	GPUParticleShaderStructs::NonSharedCopy(emitter_.sharp, desc_.emitter.sharp);
	GPUParticleShaderStructs::NonSharedCopy(emitter_.crescent, desc_.emitter.crescent);
	GPUParticleShaderStructs::NonSharedCopy(emitter_.field, desc_.emitter.field);
	GPUParticleShaderStructs::NonSharedCopy(emitter_.bullet, desc_.emitter.bullet);
	GPUParticleShaderStructs::NonSharedCopy(emitter_.bulletShape, desc_.emitter.bulletShape);
	emitter_.bullet.color = emitter_.bulletShape.color;
	for (auto& satellite : satellite_) {
		GPUParticleShaderStructs::NonSharedCopy(satellite.emitter, desc_.emitter.bulletSatellite);
	}

	isAlive_ = true;

	worldTransform_.Initialize();
	worldTransform_.translate = desc_.position;
	worldTransform_.rotate = MakeLookRotation(Normalize(desc_.velocity));
	worldTransform_.UpdateMatrix();
	preEmitterPosition_ = worldTransform_.translate;

	for (uint32_t i = 0; i < kMaxSatelliteNum; i++) {
		satellite_.at(i).worldTransform.Initialize();
		satellite_.at(i).worldTransform.parent_ = &worldTransform_;

		float angle = (2.0f * std::numbers::pi_v<float> / kMaxSatelliteNum) * i;

		satellite_.at(i).worldTransform.translate = { std::cosf(angle) * desc_.rotateOffset, std::sinf(angle) * desc_.rotateOffset, 0.0f };
	}
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
	desc_.time--;
	if (desc_.time <= 0) {
		isAlive_ = false;
		emitter_.bulletShape.isAlive = isAlive_;
		for (auto& satellite : satellite_) {
			satellite.emitter.isAlive = isAlive_;
		}
	}
	worldTransform_.translate += desc_.velocity;
	worldTransform_.rotate = worldTransform_.rotate * MakeRotateZAngleQuaternion(desc_.rotateVelocity);
	UpdateTransform();

	GPUParticleUpdate();

}

void PlayerBullet::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(worldTransform_.matWorld, viewProjection, modelHandle_, commandContext);
}

void PlayerBullet::DrawDebug() {
	collider_->DrawCollision({ 0.0f, 1.0f, 0.0f, 1.0f });
}

void PlayerBullet::GPUParticleUpdate() {
	Vector3 currentBulletPosition = MakeTranslateMatrix(worldTransform_.matWorld);
	Vector3 particleCenter = Lerp(currentBulletPosition, preEmitterPosition_, 0.5f);
	emitter_.bulletShape.emitterArea.capsule.segment.origin = currentBulletPosition;
	emitter_.bulletShape.emitterArea.capsule.segment.diff = preEmitterPosition_;

	for (auto& satellite : satellite_) {
		satellite.worldTransform.UpdateMatrix();
		satellite.emitter.emitterArea.position = MakeTranslateMatrix(satellite.worldTransform.matWorld);
		gpuParticleManager_->SetEmitter(satellite.emitter);
	}

	gpuParticleManager_->SetEmitter(emitter_.bulletShape);

	preEmitterPosition_ = worldTransform_.translate;
}

void PlayerBullet::OnCollision(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "GameObject") {
		isAlive_ = false;
		emitter_.bulletShape.isAlive = isAlive_;
		for (auto& satellite : satellite_) {
			satellite.emitter.isAlive = isAlive_;
			gpuParticleManager_->SetEmitter(satellite.emitter);
		}
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
		for (auto& satellite : satellite_) {
			satellite.emitter.isAlive = isAlive_;
			gpuParticleManager_->SetEmitter(satellite.emitter);
		}
		gpuParticleManager_->SetEmitter(emitter_.bulletShape);
	}
}

void PlayerBullet::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld));
}
