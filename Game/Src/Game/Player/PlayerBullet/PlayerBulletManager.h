#pragma once

#include <memory>
#include <list>

#include "PlayerBullet.h"

class Boss;
class PlayerUI;
class GPUParticleManager;
class CommandContext;
struct WorldTransform;
class PlayerBulletManager {
public:
	PlayerBulletManager();
	void Initialize();
	void Update();
	void DrawImGui();
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	void DrawDebug();
	void Create(const WorldTransform& worldTransform);

	void SetPlayerUI(PlayerUI* playerUI) { playerUI_ = playerUI; }
	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }
	void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
	void SetBoss(Boss* boss) { boss_ = boss; }
private:
	Boss* boss_;
	PlayerUI* playerUI_;
	GPUParticleManager* gpuParticleManager_;
	ViewProjection* viewProjection_;
	std::list<std::unique_ptr<PlayerBullet>> playerBullets_;

	int32_t bulletTime_;

	float reticleDistance_;
	float bulletSpeed_;
	uint32_t bulletLifeTime_;
	int32_t bulletCoolTime_;
	Vector3 offset_;
	float rotateVelocity_;
	float rotateOffset_;
	PlayerBullet::BulletEmitter  emitter_;
	ModelHandle modelHandle_;
};