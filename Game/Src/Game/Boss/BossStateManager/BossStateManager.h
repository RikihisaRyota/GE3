#pragma once

#include <optional>
#include <memory>

#include "Engine/Math/Random.h"
#include "Engine/Animation/Animation.h"

struct ColliderDesc;
class Boss;
class CommandContext;
class BossStateManager;
class BossState {
public:
	BossState(BossStateManager& manager, bool inTransition) : manager_(manager), inTransition_(inTransition) {}
	virtual ~BossState() {}
	virtual void Initialize() = 0;
	virtual	void SetDesc() = 0;
	virtual void Update(CommandContext& commandContext) = 0;
	//virtual void OnCollision(const ColliderDesc& colliderDesc) = 0;
	const Animation::AnimationHandle& GetAnimationHandle() const { return animationHandle_; }
	const float GetAnimationTime() const { return time_;  }
	BossStateManager& GetManager() { return manager_; }
protected:
	Animation::AnimationHandle animationHandle_;
	BossStateManager& manager_;
	float time_;
	Random::RandomNumberGenerator rnd_;
	bool inTransition_;
};

class BossStateRoot :
	public BossState {
public:
	struct JsonData {
		float allFrame;
		float transitionFrame;
	};
	using BossState::BossState;
	void Initialize() override;
	void SetDesc() override;
	void Update(CommandContext& commandContext) override;
private:
	JsonData data_;
};

class BossStateTwoHandAttack :
	public BossState {
public:
	struct JsonData {
		float allFrame;
		float transitionFrame;
	};
	using BossState::BossState;
	void Initialize() override;
	void SetDesc() override;
	void Update(CommandContext& commandContext) override;
private:
	JsonData data_;
};

class BossStateUpperAttack :
	public BossState {
public:
	struct JsonData {
		float allFrame;
		float transitionFrame;
	};
	using BossState::BossState;
	void Initialize() override;
	void SetDesc() override;
	void Update(CommandContext& commandContext) override;
private:
	JsonData data_;
};


class BossStateManager {
public:
	enum State {
		kNone,
		kRoot,
		kTwoHandAttack,
		kUpperAttack,
		kCount,
	};

	struct JsonData {
		BossStateRoot::JsonData root;
		BossStateTwoHandAttack::JsonData twoHand;
		BossStateUpperAttack::JsonData upper;
	};

	void SetBoss(Boss* boss) { boss_ = boss; }

	void Initialize();

	void Update(CommandContext& commandContext);

	const Animation::AnimationHandle GetAnimationHandle() const {
		if (preAnimationHandle_.has_value()) {
			return preAnimationHandle_.value();
		}
		return Animation::AnimationHandle();  
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
	void DrawImGui();
private:
	template<class T>
	State GetStateEnum();

	State activeStateEnum_;
	State standbyStateEnum_;
	std::unique_ptr<BossState> activeState_;
	std::unique_ptr<BossState> standbyState_;
	std::optional<float>preAnimationTime_;
	std::optional<Animation::AnimationHandle> preAnimationHandle_;
};