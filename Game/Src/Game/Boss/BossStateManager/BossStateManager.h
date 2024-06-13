#pragma once

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

	template<class T>
	void ChangeState(bool flag) {
		static_assert(std::is_base_of_v<BossState, T>, "Not inherited.");
		standbyState_ = std::make_unique<T>(*this, flag);
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

};