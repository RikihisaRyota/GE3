#pragma once

#include <array>


#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/Vector3.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/Texture/TextureHandle.h"
#include "Engine/Collision/Collider.h"

class Boss;
class CommandContext;
class PlayerBullet {
public:
	struct BulletEmitter {
		GPUParticleShaderStructs::EmitterForCPU sharp;
		GPUParticleShaderStructs::EmitterForCPU crescent;
		GPUParticleShaderStructs::EmitterForCPU bullet;
		GPUParticleShaderStructs::EmitterForCPU bulletShape;
		GPUParticleShaderStructs::FieldForCPU field;
	};
	void Create(GPUParticleManager* GPUParticleManager, const Vector3& position, const Vector3& velocity, uint32_t time,const BulletEmitter& emitter);
	void Update();
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawDebug();
	bool GetIsAlive() { return isAlive_; }

	const Vector3 GetPosition()const { return worldTransform_.translate; }
	const Quaternion GetRotate()const { return worldTransform_.rotate; }
	const float GetRadius() const { return radius_; }
	void SetBoss(Boss* boss) { boss_ = boss; }
private:
	void OnCollision(const ColliderDesc& desc);
	void UpdateTransform();

	Boss* boss_;
	GPUParticleManager* gpuParticleManager_;
	BulletEmitter emitter_;
	std::unique_ptr<SphereCollider> collider_;

	ModelHandle modelHandle_;
	TextureHandle gpuTexture_;
	WorldTransform worldTransform_;

	Vector3 velocity_;
	bool isAlive_;
	uint32_t time_;
	float radius_;
};