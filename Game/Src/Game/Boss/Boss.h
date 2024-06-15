#pragma once

#include <unordered_map>
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
	void DrawDebug(const ViewProjection& viewProjection);
private:
	void UpdateCollider();
	void UpdateGPUParticle();
	void UpdateTransform();
	void OnCollision(const ColliderDesc& desc);

	GPUParticleManager* gpuParticleManager_;

	ModelHandle bossModelHandle_;
	Animation::Animation animation_;

	WorldTransform animationTransform_;

	WorldTransform worldTransform_;

	std::unordered_map<std::string,OBBCollider*> bossCollider_;

	Vector4 colliderColor_;

	std::unique_ptr<BossStateManager> bossStateManager_;
#pragma region Collision
	float colliderSize_;
#pragma endregion

#pragma region Properties
	Vector3 offset_;
	Vector3 animationWorldTransformOffset_;
#pragma endregion

};
