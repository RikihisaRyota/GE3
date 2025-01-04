#pragma once
/**
 * @file Player.h
 * @brief Player
 */
#include <string>
#include <memory>
#include <vector>
#include <list>

#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"

#include "PlayerHP/PlayerHP.h"
#include "PlayerBullet/PlayerBullet.h"
#include "PlayerUI/PlayerUI.h"
#include "PlayerBullet/PlayerBulletManager.h"
#include "Engine/Animation/Animation.h"
#include "Engine/Collision/Collider.h"

#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"

class Boss;
class GPUParticleManager;
class CommandContext;
class Player {
public:
	// アニメーションstring
	std::vector<std::string> name_ = {
		"idle",
		"walk",
		"shootingWalk",
		"hitDamage"
	};
	// アニメーションenum
	enum State {
		kRoot,
		kWalk,
		kShootingWalk,
		kHitDamage,
		kCount,

		kNone,
	};
	Player();
	// 初期化
	void Initialize();
	// 更新
	void Update(CommandContext& commandContext);
	// Draw3D
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);
	// DrawUI
	void DrawSprite(CommandContext& commandContext);
	void DrawDebug();
	// Setter/Getter
	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) {
		gpuParticleManager_ = GPUParticleManager;
		playerBulletManager_->SetGPUParticleManager(GPUParticleManager);
	}
	void SetViewProjection(ViewProjection* viewProjection) {
		viewProjection_ = viewProjection;
		playerBulletManager_->SetViewProjection(viewProjection);
	}
	const WorldTransform& GetWorldTransform() const { return worldTransform_; }
	Vector3 GetWorldTranslate() { return MakeTranslateMatrix(worldTransform_.matWorld); }
	void DrawImGui();

	void SetBoss(Boss* boss) {
		boss_ = boss;
		playerBulletManager_->SetBoss(boss_);
	}
private:
	struct AnimationInfo {
		Animation::AnimationHandle handle;
		float animationCycle;
	};
	// UpdateTransform
	void UpdateTransform();
	// 移動
	void Move();
	// AnimationUpdate
	void AnimationUpdate(CommandContext& commandContext);
	// プレイヤーの正面を求める
	void PlayerRotate(const Vector3& vector);
	// 玉更新
	void BulletUpdate();
	// 弾発射
	void Shot();
	// 当たり判定
	void OnCollision(const ColliderDesc& desc);
	// GPUParticleSpawn
	void GPUParticleSpawn();

	Boss* boss_;
	ViewProjection* viewProjection_;
	GPUParticleManager* gpuParticleManager_;

	WorldTransform worldTransform_;


	std::unique_ptr<PlayerUI> playerUI_;
	std::unique_ptr<PlayerBulletManager> playerBulletManager_;
	std::unique_ptr<CapsuleCollider> collider_;
	std::unique_ptr<PlayerHP> playerHP_;

	ModelHandle playerModelHandle_;
	WorldTransform animationTransform_;
	Animation::Animation animation_;
	std::unordered_map<std::string, AnimationInfo> animationInfo_;
	Animation::AnimationHandle currentAnimationHandle_;
	Animation::AnimationHandle preAnimationHandle_;
	struct FootEmitter {
		GPUParticleShaderStructs::EmitterForCPU fugitiveDust;
	};
	FootEmitter footEmitter_;
	GPUParticleShaderStructs::MeshEmitterForCPU meshEmitterDesc_;
	GPUParticleShaderStructs::VertexEmitterForCPU vertexEmitterDesc_;

	State state_;
	State preState_;
	std::optional<State> tmpState_;
	float animationTime_;
	float transitionTime_;
	bool onTransition_;
	// 遷移なしでアニメーションスタート
	bool immediatelyTransition_;

	Vector4 colliderColor_;

	Vector3 velocity_;
	Vector3 acceleration_;
	float gravity_;
	Vector3 knockBackStartPos_;
	Vector3 knockBackEndPos_;
	float knockBack_;

	float transitionCycle_;

	float walkSpeed_;
	float shootingWalkSpeed_;

	float colliderRadius_;
};