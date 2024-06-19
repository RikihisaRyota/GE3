#pragma once

#include <array>

#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/Texture/TextureHandle.h"
#include "Engine/Collision/Collider.h"

class CommandContext;
class PlayerBullet {
public:
	void Create(GPUParticleManager* GPUParticleManager, const Vector3& position, const Vector3& velocity, uint32_t time);
	void Update();
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawDebug(const ViewProjection& viewProjection);
	bool GetIsAlive() { return isAlive_; }
private:
	void OnCollision(const ColliderDesc& desc);
	void UpdateTransform();

	GPUParticleManager* gpuParticleManager_;

	std::unique_ptr<OBBCollider> collider_;

	ModelHandle modelHandle_;
	TextureHandle gpuTexture_;
	WorldTransform worldTransform_;

	Vector3 velocity_;
	bool isAlive_;
	uint32_t time_;
};