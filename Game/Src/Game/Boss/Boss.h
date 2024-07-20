#pragma once

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


class GPUParticleManager;
class CommandContext;
class Boss {
public:
	Boss();
	void Initialize();
	void Update(CommandContext& commandContext);
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { 
		gpuParticleManager_ = GPUParticleManager;
		bossStateManager_->SetGPUParticleManager(gpuParticleManager_);
	}

	Animation::Animation* GetAnimation() { return &animation_; }
	const ModelHandle& GetModelHandle() { return bossModelHandle_; }
	void DrawImGui();
	void DrawDebug();
	std::unique_ptr<SphereCollider>& GetCollider() { return collider_; }

	Vector3 GetWorldTranslate();
	const Matrix4x4& GetWorldMatrix()const { return worldTransform_.matWorld; }
	const GPUParticleShaderStructs::VertexEmitterDesc& GetVertexEmitter()const { return vertexEmitterDesc_; }
private:
	void UpdateCollider();
	void UpdateGPUParticle(CommandContext& commandContext);
	void UpdateTransform();
	void OnCollisionBody(const ColliderDesc& desc);
	void OnCollisionAttack(const ColliderDesc& desc);

	GPUParticleManager* gpuParticleManager_;
	TextureHandle gpuTexture_;


	ModelHandle bossModelHandle_;
	Animation::Animation animation_;

	WorldTransform animationTransform_;
	/// <summary>
	GPUParticleShaderStructs::ParticleLifeSpan particleLifeSpan;
	GPUParticleShaderStructs::ScaleAnimation scaleAnimation;
	GPUParticleShaderStructs::FloatMinMax distanceFactor;

	float t_;
	ModelHandle testModelHandle_;

	/// </summary>
	GPUParticleShaderStructs::MeshEmitterDesc meshEmitterDesc_;
	GPUParticleShaderStructs::VertexEmitterDesc vertexEmitterDesc_;
	GPUParticleShaderStructs::TransformEmitter transformEmitter_;

	WorldTransform worldTransform_;

	std::unique_ptr<BossStateManager> bossStateManager_;
	std::unique_ptr<BossHP> bossHP_;

	std::unique_ptr<SphereCollider> collider_;

#pragma region Properties
	Vector3 offset_;
	Vector3 animationWorldTransformOffset_;
#pragma endregion

};
