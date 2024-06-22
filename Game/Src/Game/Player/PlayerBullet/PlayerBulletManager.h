#pragma once

#include <memory>
#include <list>

#include "PlayerBullet.h"


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
	void DrawDebug(const ViewProjection& viewProjection);
	void Create(const WorldTransform& worldTransform);

	void SetPlayerUI(PlayerUI* playerUI) { playerUI_ = playerUI; }
	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }
	void SetViewProjection(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
private:
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
};