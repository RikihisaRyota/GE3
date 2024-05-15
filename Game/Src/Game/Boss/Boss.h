#pragma once
#include <memory>
#include <list>

#include "Engine/Animation/Animation.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/Collision/Collider.h"

#include "Engine/GPUParticleManager/GPUParticleManager.h"

class CommandContext;
class Boss {
public:
	Boss();
	void Initialize();
	void Update();
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
private:
	void UpdateTransform();
	void OnCollision(const ColliderDesc& desc);

	ModelHandle bossModelHandle_;
	Animation::Animation animation_;
	float animationTime_;
	WorldTransform animationTransform_;

	WorldTransform worldTransform_;

	OBBCollider* collider_;

	Vector4 colliderColor_;
};
