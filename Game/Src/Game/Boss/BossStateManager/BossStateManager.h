#pragma once

#include <optional>
#include <memory>

#include "Engine/Math/Random.h"
#include "Engine/Animation/Animation.h"
#include "Engine/Model/ModelHandle.h"
#include "Engine/GPUParticleManager/GPUParticle/GPUParticleShaderStructs.h"
#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/Collision/Collider.h"

struct ColliderDesc;
class Boss;
class CommandContext;
class BossStateManager;
class BossState {
public:
	BossState(BossStateManager& manager, bool inTransition) : manager_(manager), inTransition_(inTransition) {}
	virtual ~BossState() {}
	virtual void Initialize(CommandContext& commandContext) = 0;
	virtual	void SetDesc() = 0;
	virtual void Update(CommandContext& commandContext) = 0;
	virtual void DebugDraw()=0;
	//virtual void OnCollision(const ColliderDesc& colliderDesc) = 0;
	const Animation::AnimationHandle& GetAnimationHandle() const { return animationHandle_; }
	const ModelHandle& GetModelHandle() const { return modelHandle_; }
	const float GetAnimationTime() const { return time_; }
	const WorldTransform& GetWorldTransform()const { return worldTransform_; }
	BossStateManager& GetManager() { return manager_; }
	bool GetInTransition() { return inTransition_; }
protected:
	BossStateManager& manager_;
	Random::RandomNumberGenerator rnd_;
	bool inTransition_;

	Animation::AnimationHandle animationHandle_;
	ModelHandle modelHandle_;
	WorldTransform worldTransform_;
	float time_;
};

class BossStateRoot :
	public BossState {
public:
	struct JsonData {
		GPUParticleShaderStructs::TransformModelEmitterForCPU transformEmitter;
		float allFrame;
		float transitionFrame;
	};
	using BossState::BossState;
	void Initialize(CommandContext& commandContext) override;
	void SetDesc() override;
	void Update(CommandContext& commandContext) override;
	void DebugDraw()override;
private:
	JsonData data_;
};
//
//class BossStateTwoHandAttack :
//	public BossState {
//public:
//	struct JsonData {
//		float allFrame;
//		float transitionFrame;
//	};
//	using BossState::BossState;
//	void Initialize() override;
//	void SetDesc() override;
//	void Update(CommandContext& commandContext) override;
//private:
//	JsonData data_;
//};

class BossStateRushAttack :
	public BossState {
public:
	struct JsonData {
		GPUParticleShaderStructs::VertexEmitterForCPU  trainEmitter;
		GPUParticleShaderStructs::VertexEmitterForCPU  railEmitter;
		GPUParticleShaderStructs::VertexEmitterForCPU  transformRailVertexEmitter;
		GPUParticleShaderStructs::TransformModelEmitterForCPU  transformTrainEmitter;
		GPUParticleShaderStructs::TransformAreaEmitterForCPU  transformRailEmitter;
		OBB collider;
		float frontAndBackOffset;
		Vector3 start;
		Vector3 end;
		float allFrame;
		float transitionFrame;
	};
	using BossState::BossState;
	void Initialize(CommandContext& commandContext) override;
	void SetDesc() override;
	void Update(CommandContext& commandContext) override;
	void DebugDraw()override;
private:
	enum AttackLocation {
		kRight = 1 << 0,  // 0001
		kLeft = 1 << 1,   // 0010
		kBack = 1 << 2,   // 0100
		kFront = 1 << 3,  // 1000
	};
	uint32_t attackLocation_;
	void OnCollision(const ColliderDesc& collisionInfo);
	void UpdateTransform();
	void SetLocation();

	ModelHandle railModelHandle_;
	WorldTransform railWorldTransform_;
	JsonData data_;
	std::unique_ptr<OBBCollider> collider_;
};


class BossStateSmashAttack :
	public BossState {
public:
	struct JsonData {
		GPUParticleShaderStructs::VertexEmitterForCPU  smashEmitter;
		GPUParticleShaderStructs::TransformModelEmitterForCPU  transformEmitter;
		OBB collider;
		float y;
		uint32_t smashCount;
		float allFrame;
		float transitionFrame;
	};
	using BossState::BossState;
	void Initialize(CommandContext& commandContext) override;
	void SetDesc() override;
	void Update(CommandContext& commandContext) override;
	void DebugDraw()override;
private:
	struct SmashDesc {
		Vector3 start;
		Vector3 end;
		WorldTransform transform;
		std::unique_ptr<OBBCollider> collider;
		GPUParticleShaderStructs::VertexEmitterForCPU  emitter;
	};
	void OnCollision(const ColliderDesc& collisionInfo);
	void UpdateTransform();
	void SetLocation();
	void CreateSmash();
	std::vector<SmashDesc> smash_;
	JsonData data_;
};

class BossStateManager {
public:
	std::vector<std::string> stateNames_{
		"None",
		"Root",
		"RushAttack",
		"SmashAttack",
	};
	enum State {
		kNone,
		kRoot,
		kRushAttack,
		kSmashAttack,
		kCount,
	};

	struct JsonData {
		BossStateRoot::JsonData root;
		BossStateRushAttack::JsonData rushAttack;
		BossStateSmashAttack::JsonData smashAttack;
	};

	void SetBoss(Boss* boss) { boss_ = boss; }
	void SetGPUParticleManager(GPUParticleManager* gpuParticleManager) { gpuParticleManager_ = gpuParticleManager; }

	void Initialize();

	void Update(CommandContext& commandContext);

	const Animation::AnimationHandle GetAnimationHandle() const {
		if (preAnimationHandle_.has_value()) {
			return preAnimationHandle_.value();
		}
		return Animation::AnimationHandle();
	}

	const ModelHandle GetModelHandle() const {
		if (preModelHandle_.has_value()) {
			return preModelHandle_.value();
		}
		return ModelHandle();
	}

	const WorldTransform GetWorldTransform() const {
		if (preWorldTransform_.has_value()) {
			return preWorldTransform_.value();
		}
		return WorldTransform();
	}

	const float GetAnimationTime() const {
		if (preAnimationTime_.has_value()) {
			return preAnimationTime_.value();
		}
		return 0.0f;
	}

	template<class T>
	void ChangeState() {
		bool inTransition = false;
		if (activeState_) {
			preAnimationHandle_ = activeState_->GetAnimationHandle();
			preAnimationTime_ = activeState_->GetAnimationTime();
			preModelHandle_ = activeState_->GetModelHandle();
			preWorldTransform_ = activeState_->GetWorldTransform();
			if (preAnimationHandle_) {
				inTransition = true;
			}
		}
		static_assert(std::is_base_of_v<BossState, T>, "Not inherited.");
		standbyState_ = std::make_unique<T>(*this, inTransition);
		standbyStateEnum_ = GetStateEnum<T>();
	}
	JsonData jsonData_;
	Boss* boss_;
	GPUParticleManager* gpuParticleManager_;
	void DrawImGui();
	const State& GetPreState() { return preStateEnum_; }
	const State& GetCurrentState() { return activeStateEnum_; }
	bool GetInTransition() {
		if (activeState_) {
			return activeState_->GetInTransition();
		}
		return false;
	}
private:
	template<class T>
	State GetStateEnum();

	State activeStateEnum_;
	State standbyStateEnum_;
	State preStateEnum_;
	std::unique_ptr<BossState> activeState_;
	std::unique_ptr<BossState> standbyState_;
	std::optional<float>preAnimationTime_;
	std::optional<Animation::AnimationHandle> preAnimationHandle_;
	std::optional<ModelHandle> preModelHandle_;
	std::optional<WorldTransform> preWorldTransform_;
};