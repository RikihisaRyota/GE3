#pragma once
#include <memory>
#include <list>

#include "Engine/Animation/Animation.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/Collision/Collider.h"

#include "BossStateManager/BossStateManager.h"

class GPUParticleManager;
class CommandContext;
class Boss {
public:
	Boss();
	void Initialize();
	void Update(CommandContext& commandContext);
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }

	Animation::Animation* GetAnimation() { return &animation_; }
	const ModelHandle& GetModelHandle() { return bossModelHandle_; }
	void DrawImGui();
private:
	void UpdateGPUParticle();
	void UpdateTransform();
	void OnCollision(const ColliderDesc& desc);

	GPUParticleManager* gpuParticleManager_;

	ModelHandle bossModelHandle_;
	Animation::Animation animation_;

	WorldTransform animationTransform_;

	WorldTransform worldTransform_;

	//OBBCollider* collider_;

	Vector4 colliderColor_;

	std::unique_ptr<BossStateManager> bossStateManager_;
};
