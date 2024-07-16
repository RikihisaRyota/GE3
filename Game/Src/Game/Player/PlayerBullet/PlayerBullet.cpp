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
	emitter_ = emitter;
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
	collider_->SetCollisionMask(CollisionAttribute::GameObject | CollisionAttribute::BossBody);
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
	ModelManager::GetInstance()->Draw(worldTransform_.matWorld, viewProjection, modelHandle_, commandContext);
}

void PlayerBullet::DrawDebug(){
	collider_->DrawCollision({ 0.0f, 1.0f, 0.0f, 1.0f });
}

void PlayerBullet::OnCollision(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "GameObject") {
		isAlive_ = false;
	}
	if (desc.collider->GetName().find("Boss") != std::string::npos) {
		std::string jointName = EraseName(desc.collider->GetName(), "Boss_");
		float ratio = float(boss_->GetBodyEmitters()[jointName].createParticleNum) / float(boss_->GetInitializeParticleNum()[jointName]);
		ratio -= 0.05f;
		boss_->GetBodyEmitters()[jointName].createParticleNum = uint32_t(boss_->GetInitializeParticleNum()[jointName] * ratio);
		emitter_.sharp.emitterArea.sphere.position = MakeTranslateMatrix(worldTransform_.matWorld);
		emitter_.crescent.emitterArea.sphere.position = MakeTranslateMatrix(worldTransform_.matWorld);
		gpuParticleManager_->SetEmitter(emitter_.crescent);
		gpuParticleManager_->SetEmitter(emitter_.sharp);
		isAlive_ = false;
	}
}

void PlayerBullet::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld));
}
