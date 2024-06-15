#pragma once

#include <memory>
#include <list>

#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"

#include "PlayerBullet.h"
#include "Engine/Animation/Animation.h"
#include "Engine/Collision/Collider.h"

class GPUParticleManager;
class CommandContext;
class Player {
public:
	Player();

	void Initialize();

	void Update(CommandContext& commandContext);

	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawDebug(const ViewProjection& viewProjection);

	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }
	void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
	void DrawImGui();
private:
	static const uint32_t kBulletTime = 120;
	static const uint32_t kBulletCoolTime = 15;

	void UpdateTransform();

	void Move();
	
	void AnimationUpdate(CommandContext& commandContext);
	void PlayerRotate(const Vector3& vector);

	void BulletUpdate();
	void Shot();
	void OnCollision(const ColliderDesc& desc);

	void GPUParticleSpawn();

	ViewProjection* viewProjection_;
	GPUParticleManager* gpuParticleManager_;

	WorldTransform worldTransform_;

	OBBCollider* collider_;

	ModelHandle playerModelHandle_;
	WorldTransform animationTransform_;
	Animation::Animation animation_;
	Animation::AnimationHandle walkHandle_;
	float animationTime_;

	Vector4 colliderColor_;

	uint32_t bulletTime_;
	std::list<std::unique_ptr<PlayerBullet>> playerBullets_;
};