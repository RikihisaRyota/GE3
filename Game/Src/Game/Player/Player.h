#pragma once

#include <memory>
#include <list>

#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"

#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "PlayerBullet.h"
#include "Engine/Animation/Animation.h"


class CommandContext;
class Player {
public:
	Player();

	void Initialize();

	void Update();

	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }
	void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
private:
	static const uint32_t kBulletTime = 120;
	static const uint32_t kBulletCoolTime = 15;

	void Move();
	
	void AnimationUpdate();
	void PlayerRotate(const Vector3& vector);

	void BulletUpdate();
	void Shot();

	ViewProjection* viewProjection_;
	GPUParticleManager* gpuParticleManager_;

	ModelHandle playerModelHandle_;
	WorldTransform worldTransform_;
	WorldTransform animationTransform_;
	Animation::Animation animation_;
	float animationTime_;

	uint32_t bulletTime_;
	std::list<std::unique_ptr<PlayerBullet>> playerBullets_;
};