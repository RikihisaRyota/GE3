#pragma once

#include <memory>
#include <list>

#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"

#include "PlayerBullet.h"
#include "PlayerUI/PlayerUI.h"
#include "Engine/Animation/Animation.h"
#include "Engine/Collision/Collider.h"

class GPUParticleManager;
class CommandContext;
class Player {
public:
	enum State {
		kRoot,
		kWalk,
		kShootWalk,
		kCount,
	};
	Player();

	void Initialize();

	void Update(CommandContext& commandContext);

	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawSprite(CommandContext& commandContext);
	void DrawDebug(const ViewProjection& viewProjection);

	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }
	void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
	void DrawImGui();
private:
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
	Animation::AnimationHandle shootWalkHandle_;
	Animation::AnimationHandle idleHandle_;
	Animation::AnimationHandle currentAnimationHandle_;
	Animation::AnimationHandle preAnimationHandle_;
	State state_;
	State preState_;
	float animationTime_;
	float transitionTime_;
	bool onTransition_;

	Vector4 colliderColor_;

	uint32_t bulletTime_;
	std::list<std::unique_ptr<PlayerBullet>> playerBullets_;

	std::unique_ptr<PlayerUI> playerUI_;

	float reticleDistance_;
	float bulletSpeed_;
	uint32_t bulletLifeTime_;
	uint32_t bulletCoolTime_;
	
	float idleAnimationCycle_;
	float walkAnimationCycle_;
	float shootWalkAnimationCycle_;
	float transitionCycle_;
};