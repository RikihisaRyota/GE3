#pragma once
/**
 * @file Boss.h
 * @brief ボス
 */
#include <unordered_map>
#include <memory>
#include <list>

#include "Engine/Animation/Animation.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/Math/WorldTransform.h"
#include "Engine/Math/ViewProjection.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/Collision/Collider.h"

#include "BossStateManager/BossStateManager.h"
#include "BossHP/BossHP.h"


class Player;
class GPUParticleManager;
class CommandContext;
class Boss {
public:
	// コンストラクタ
	Boss();
	// 初期化
	void Initialize();
	// 更新
	void Update();
	// 描画
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	// Setter/Getter
	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) {
		gpuParticleManager_ = GPUParticleManager;
		bossStateManager_->SetGPUParticleManager(gpuParticleManager_);
	}
	void SetPlayer(Player* player) {
		player_ = player;
		bossStateManager_->SetPlayer(player_);
	}

	Engine::Animation::Animation* GetAnimation() { return &animation_; }
	const ModelHandle& GetModelHandle() { return bossModelHandle_; }
	// デバック
	void DrawImGui();
	void DrawDebug();
	std::unique_ptr<SphereCollider>& GetCollider() { return collider_; }

	Vector3 GetWorldTranslate()const;
	const Matrix4x4& GetWorldMatrix()const { return worldTransform_.matWorld; }
	const GPUParticleShaderStructs::VertexEmitterForCPU& GetVertexEmitter()const { return vertexEmitterDesc_; }
private:
	// 当たり判定更新
	void UpdateCollider();
	// GPUParticle更新
	void UpdateGPUParticle();
	// Transform更新
	void UpdateTransform();
	// ボスの体との当たり判定
	void OnCollisionBody(const ColliderDesc& desc);
	// ボスの攻撃とプレイヤーの当たり判定
	void OnCollisionAttack(const ColliderDesc& desc);

	Player* player_;
	GPUParticleManager* gpuParticleManager_;
	TextureHandle gpuTexture_;


	ModelHandle bossModelHandle_;
	Engine::Animation::Animation animation_;

	WorldTransform collisionTransform_;
	WorldTransform animationTransform_;

	GPUParticleShaderStructs::MeshEmitterForCPU meshEmitterDesc_;
	GPUParticleShaderStructs::VertexEmitterForCPU vertexEmitterDesc_;
	GPUParticleShaderStructs::TransformModelEmitterForCPU  transformEmitter_;

	WorldTransform worldTransform_;

	std::unique_ptr<BossStateManager> bossStateManager_;
	std::unique_ptr<BossHP> bossHP_;

	std::unique_ptr<SphereCollider> collider_;
#pragma region Properties
	Vector3 offset_;
	Vector3 collisionWorldTransformOffset_;
	Vector3 animationWorldTransformOffset_;
#pragma endregion

};
