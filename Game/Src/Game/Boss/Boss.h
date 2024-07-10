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
private:
	struct BossCollider {
		std::unique_ptr<CapsuleCollider> body;
		std::unique_ptr<CapsuleCollider> attack;
		Vector4 color;
	};
public:
	Boss();
	void Initialize();
	void Update(CommandContext& commandContext);
	void Draw(const ViewProjection& viewProjection, CommandContext& commandContext);

	void SetGPUParticleManager(GPUParticleManager* GPUParticleManager) { gpuParticleManager_ = GPUParticleManager; }

	Animation::Animation* GetAnimation() { return &animation_; }
	const ModelHandle& GetModelHandle() { return bossModelHandle_; }
	void DrawImGui();
	void DrawDebug();
	std::unordered_map<std::string, std::unique_ptr<BossCollider>>& GetCollider() { return bossCollider_; }
	std::unordered_map<std::string, GPUParticleShaderStructs::EmitterForCPU>& GetEmitters() { return emitters_; }
	std::unordered_map<std::string, uint32_t>& GetInitializeParticleNum() { return initializeParticleNum_; }
	const std::vector<std::string>& GetColliderType(const std::string& name) { return colliderType_[name]; }

	const GPUParticleShaderStructs::EmitterColor& GetAttackColor() { return attackColor_; }
	const GPUParticleShaderStructs::EmitterColor& GetDefaultColor() { return defaultColor_; }
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
	/// </summary>
	GPUParticleShaderStructs::MeshEmitterDesc meshEmitterDesc_;
	GPUParticleShaderStructs::VertexEmitterDesc vertexEmitterDesc_;

	WorldTransform worldTransform_;

	std::unordered_map<std::string, std::unique_ptr<BossCollider>> bossCollider_;

	std::unique_ptr<BossStateManager> bossStateManager_;
	std::unique_ptr<BossHP> bossHP_;
#pragma region Collision
	std::unordered_map<std::string, float> colliderSize_;
	std::unordered_map<std::string, GPUParticleShaderStructs::EmitterForCPU> emitters_;
	std::unordered_map<std::string, uint32_t> initializeParticleNum_;
	GPUParticleShaderStructs::EmitterColor attackColor_;
	GPUParticleShaderStructs::EmitterColor defaultColor_;

	std::unordered_map<std::string, std::vector<std::string>> colliderType_;
	std::unordered_map<std::string, int> selectedNodeNameIndices_;
	std::unordered_map<std::string, int> selectedEntryNodeNameIndices_;
#pragma endregion

#pragma region Properties
	Vector3 offset_;
	Vector3 animationWorldTransformOffset_;
#pragma endregion

};
