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
		GPUParticleShaderStructs::EmitterForCPU bulletSatellite;
		GPUParticleShaderStructs::FieldForCPU field;
	};
	struct BulletDesc {
		Vector3 position;
		Vector3 velocity;
		uint32_t time;
		float rotateVelocity;
		float rotateOffset;
		BulletEmitter emitter;
	};
	// 生成
	void Create(GPUParticleManager* GPUParticleManager, const BulletDesc& desc);
	// 更新
	void Update();
	// 描画
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	// デバック
	void DrawDebug();
	// Particle更新
	void GPUParticleUpdate();
	// Setter/Getter
	bool GetIsAlive() { return isAlive_; }

	const Vector3 GetPosition()const { return worldTransform_.translate; }
	const Quaternion GetRotate()const { return worldTransform_.rotate; }
	const float GetRadius() const { return radius_; }
	void SetBoss(Boss* boss) { boss_ = boss; }
private:
	static const uint32_t kMaxSatelliteNum = 3;
	void OnCollision(const ColliderDesc& desc);
	void UpdateTransform();

	struct Satellite {
		WorldTransform worldTransform;
		GPUParticleShaderStructs::EmitterForCPU emitter;
	};

	Boss* boss_;
	GPUParticleManager* gpuParticleManager_;
	BulletDesc desc_;
	std::unique_ptr<SphereCollider> collider_;

	BulletEmitter emitter_;

	ModelHandle modelHandle_;
	TextureHandle gpuTexture_;
	WorldTransform worldTransform_;
	Vector3 preEmitterPosition_;

	bool isAlive_;
	float radius_;

	std::array<Satellite, kMaxSatelliteNum>satellite_;
};