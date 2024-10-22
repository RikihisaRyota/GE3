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
class Player;
class BossState {
public:
	BossState(BossStateManager& manager, bool inTransition) : manager_(manager), inTransition_(inTransition) {}
	virtual ~BossState() {}
	virtual void Initialize(CommandContext& commandContext) = 0;
	virtual	void SetDesc() = 0;
	virtual void Update(CommandContext& commandContext) = 0;
	virtual void DebugDraw() = 0;
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
		GPUParticleShaderStructs::EmitterForCPU disguisingEmitter;
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

class BossStateRushAttack :
	public BossState {
public:
	struct JsonData {
		GPUParticleShaderStructs::FieldForCPU headField;
		GPUParticleShaderStructs::VertexEmitterForCPU  trainEmitter;
		GPUParticleShaderStructs::VertexEmitterForCPU  railEmitter;
		GPUParticleShaderStructs::VertexEmitterForCPU  transformRailVertexEmitter;
		GPUParticleShaderStructs::TransformModelEmitterForCPU  transformTrainEmitter;
		GPUParticleShaderStructs::TransformAreaEmitterForCPU  transformRailEmitter;
		GPUParticleShaderStructs::TransformAreaEmitterForCPU  areaTrainEmitter;
		GPUParticleShaderStructs::EmitterForCPU trainSmokeEmitter;
		GPUParticleShaderStructs::EmitterForCPU smokeEmitter;
		GPUParticleShaderStructs::EmitterForCPU headEmitter;
		OBB collider;
		float frontAndBackOffset;
		Vector3 headOffset;
		Vector3 smokeOffset;
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
		GPUParticleShaderStructs::VertexEmitterForCPU  smashAfterimageEmitter;
		GPUParticleShaderStructs::EmitterForCPU smashHeadEmitter;
		GPUParticleShaderStructs::TransformModelEmitterForCPU  transformEmitter;
		OBB collider;
		Vector3 headOffset;
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
		float time;
		bool canNextMove;
		bool isFinish;

		void Update(float startPosY, float allFrame);
	};
	void OnCollision(const ColliderDesc& collisionInfo);
	void UpdateTransform();
	void SetLocation();
	void CreateSmash();
	std::vector<SmashDesc> smash_;
	JsonData data_;
};

class BossStateHomingAttack :
	public BossState {
public:
	struct JsonData {
		GPUParticleShaderStructs::VertexEmitterForCPU  homingEmitter;
		GPUParticleShaderStructs::TransformModelEmitterForCPU transformEmitter;
		GPUParticleShaderStructs::EmitterForCPU bulletEmitter;
		GPUParticleShaderStructs::FieldForCPU bulletField;
		GPUParticleShaderStructs::EmitterForCPU fireEmitter;

		GPUParticleShaderStructs::EmitterForCPU homingExplosionEmitter;
		GPUParticleShaderStructs::FieldForCPU homingExplosionField;

		Vector3 start;
		float allFrame;
		float transitionFrame;

		Vector3 bulletOffset;
		Vector3 bulletHeightOffset;
		int createBulletInterval;
		float bulletSpeed;
		float bulletRadius;
	};
	using BossState::BossState;
	void Initialize(CommandContext& commandContext) override;
	void SetDesc() override;
	void Update(CommandContext& commandContext) override;
	void DebugDraw()override;
private:
	void InitializeBullet();
	void UpdateBullet();
	class Bullet {
	public:
		void Initialize(float radius);
		void Create(const Vector3& start, const Vector3& end, const Vector3& offset, float speed);
		void Update();
		void DebugDraw();
		bool GetIsAlive() const { return isAlive_; }
		const WorldTransform& GetWorldTransform() const { return worldTransform_; }
		void SetEmitter(
			const GPUParticleShaderStructs::EmitterForCPU& bulletEmitter,
			const GPUParticleShaderStructs::EmitterForCPU& explosionEmitter) {
			GPUParticleShaderStructs::NonSharedCopy(bulletEmitter_, bulletEmitter);
			GPUParticleShaderStructs::NonSharedCopy(explosionEmitter_, explosionEmitter);
		}
		const GPUParticleShaderStructs::EmitterForCPU& GetBulletEmitter() const { return bulletEmitter_; }
		const GPUParticleShaderStructs::EmitterForCPU& GetExplosionEmitter() const { return explosionEmitter_; }
		void SetField(
			const GPUParticleShaderStructs::FieldForCPU& field,
			const GPUParticleShaderStructs::FieldForCPU& explosionField) {
			GPUParticleShaderStructs::NonSharedCopy(bulletField_, field);
			GPUParticleShaderStructs::NonSharedCopy(explosionField_, explosionField);
		}
		const GPUParticleShaderStructs::FieldForCPU& GetBulletField() const { return bulletField_; }
		const GPUParticleShaderStructs::FieldForCPU& GetExplosionFieldField() const { return explosionField_; }
	private:
		void OnCollision(const ColliderDesc& desc);
		GPUParticleShaderStructs::EmitterForCPU bulletEmitter_;
		GPUParticleShaderStructs::FieldForCPU bulletField_;
		GPUParticleShaderStructs::EmitterForCPU explosionEmitter_;
		GPUParticleShaderStructs::FieldForCPU explosionField_;
		std::unique_ptr<SphereCollider> collider;
		bool isAlive_;
		float time_;
		float allFrame_;
		Vector3 end_;
		Vector3 start_;
		Vector3 bulletHeightOffset_;

		ModelHandle bulletModel_;
		WorldTransform worldTransform_;
	};
	std::array<std::pair<bool, std::unique_ptr<Bullet>>, 5> bullets_;
	int createBulletTime_;
	JsonData data_;
};


class BossStateManager {
public:
	std::vector<std::string> stateNames_{
		"None",
		"Root",
		"RushAttack",
		"SmashAttack",
		"Homing",
	};
	enum State {
		kNone,
		kRoot,
		kRushAttack,
		kSmashAttack,
		kHomingAttack,
		kCount,
	};

	struct JsonData {
		BossStateRoot::JsonData root;
		BossStateRushAttack::JsonData rushAttack;
		BossStateSmashAttack::JsonData smashAttack;
		BossStateHomingAttack::JsonData homingAttack;
	};

	void SetBoss(Boss* boss) { boss_ = boss; }
	void SetPlayer(Player* player) { player_ = player; }
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
	Player* player_;
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