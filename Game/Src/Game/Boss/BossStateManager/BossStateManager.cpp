#include "BossStateManager.h"

#include "../Boss.h"
#include "Engine/Json/JsonUtils.h"
#include "Engine/ImGui/ImGuiManager.h"

void BossStateRoot::Initialize() {
	SetDesc();
	animationHandle_ = manager_.boss_->GetAnimation()->GetAnimationHandle("idle");
	time_ = 0.0f;
}

void BossStateRoot::SetDesc() {
	data_ = manager_.jsonData_.root;
}

void BossStateRoot::Update(CommandContext& commandContext) {
	if (time_ >= 1.0f && inTransition_) {
		inTransition_ = false;
		time_ = 0.0f;
	}

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
	}
	else {
		time_ += 1.0f / data_.allFrame;
		time_ = std::fmod(time_, 1.0f);
	}

	auto boss = manager_.boss_;
	auto animation = manager_.boss_->GetAnimation();
	if (inTransition_) {
		animation->Update(manager_.GetAnimationHandle(), manager_.GetAnimationTime(), animationHandle_, 0.0f, time_, commandContext, boss->GetModelHandle());
	}
	else {
		animation->Update(animationHandle_, time_, commandContext, boss->GetModelHandle());
	}
}

void BossStateTwoHandAttack::Initialize() {
	SetDesc();
	animationHandle_ = manager_.boss_->GetAnimation()->GetAnimationHandle("twoHandAttack");
	time_ = 0.0f;
}

void BossStateTwoHandAttack::SetDesc() {
	data_ = manager_.jsonData_.twoHand;
}

void BossStateTwoHandAttack::Update(CommandContext& commandContext) {
	auto boss = manager_.boss_;
	auto animation = boss->GetAnimation();

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
		if (time_ >= 1.0f) {
			inTransition_ = false;
			time_ = 0.0f;
		}
	}

	if (!inTransition_) {
		time_ += 1.0f / data_.allFrame;
	}

	time_ = std::clamp(time_, 0.0f, 1.0f);

	if (inTransition_) {
		animation->Update(manager_.GetAnimationHandle(), manager_.GetAnimationTime(), animationHandle_, 0.0f, time_, commandContext, boss->GetModelHandle());
	}
	else {
		animation->Update(animationHandle_, time_, commandContext, boss->GetModelHandle());
	}

	if (time_ >= 1.0f && !inTransition_) {
		manager_.ChangeState<BossStateRoot>();
	}
}

void BossStateUpperAttack::Initialize() {
	SetDesc();
	animationHandle_ = manager_.boss_->GetAnimation()->GetAnimationHandle("upperAttack");
	time_ = 0.0f;
}

void BossStateUpperAttack::SetDesc() {
	data_ = manager_.jsonData_.upper;
}

void BossStateUpperAttack::Update(CommandContext& commandContext) {
	auto boss = manager_.boss_;
	auto animation = boss->GetAnimation();

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
		if (time_ >= 1.0f) {
			inTransition_ = false;
			time_ = 0.0f;
		}
	}

	if (!inTransition_) {
		time_ += 1.0f / data_.allFrame;
	}

	time_ = std::clamp(time_, 0.0f, 1.0f);

	if (inTransition_) {
		animation->Update(manager_.GetAnimationHandle(), manager_.GetAnimationTime(), animationHandle_, 0.0f, time_, commandContext, boss->GetModelHandle());
	}
	else {
		animation->Update(animationHandle_, time_, commandContext, boss->GetModelHandle());
	}

	if (time_ >= 1.0f && !inTransition_) {
		manager_.ChangeState<BossStateRoot>();
	}
}


void BossStateManager::Initialize() {
	JSON_OPEN("Resources/Data/Boss/BossState.json");
	JSON_OBJECT("StateRoot");
	JSON_LOAD(jsonData_.root.allFrame);
	JSON_LOAD(jsonData_.root.transitionFrame);
	JSON_ROOT();
	JSON_OBJECT("StateTwoHandAttack");
	JSON_LOAD(jsonData_.twoHand.allFrame);
	JSON_LOAD(jsonData_.twoHand.transitionFrame);
	JSON_ROOT();
	JSON_OBJECT("StateUpperAttack");
	JSON_LOAD(jsonData_.upper.allFrame);
	JSON_LOAD(jsonData_.upper.transitionFrame);
	JSON_ROOT();
	JSON_CLOSE();
	activeStateEnum_ = kRoot;
	standbyStateEnum_ = kRoot;
	ChangeState<BossStateRoot>();
}

void BossStateManager::Update(CommandContext& commandContext) {
	if (standbyState_) {
		activeState_ = std::move(standbyState_);
		activeState_->Initialize();
		activeStateEnum_ = standbyStateEnum_;
		standbyStateEnum_ = kNone;
	}

	if (activeState_) {
		activeState_->Update(commandContext);
	}
}

void BossStateManager::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Boss")) {
		if (ImGui::TreeNode("BossStateManager")) {
			// ステートを変更するImGui::Comboの作成
			static const char* stateNames[] = { "None", "Root", "TwoHandAttack", "UpperAttack" };
			int currentState = static_cast<int>(activeStateEnum_);

			// ステートを変更するImGui::Comboの作成
			if (ImGui::Combo("Change State", &currentState, stateNames, IM_ARRAYSIZE(stateNames))) {
				switch (currentState) {
				case kRoot:
					ChangeState<BossStateRoot>();
					break;
				case kTwoHandAttack:
					ChangeState<BossStateTwoHandAttack>();
					break;
				case kUpperAttack:
					ChangeState<BossStateUpperAttack>();
					break;
				default:
					break;
				}
			}
			if (ImGui::TreeNode("Root")) {
				ImGui::DragFloat("全体フレーム", &jsonData_.root.allFrame, 0.1f);
				ImGui::DragFloat("遷移フレーム", &jsonData_.root.transitionFrame, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("TwoHandAttack")) {
				ImGui::DragFloat("全体フレーム", &jsonData_.twoHand.allFrame, 0.1f);
				ImGui::DragFloat("遷移フレーム", &jsonData_.twoHand.transitionFrame, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("UpperAttack")) {
				ImGui::DragFloat("全体フレーム", &jsonData_.upper.allFrame, 0.1f);
				ImGui::DragFloat("遷移フレーム", &jsonData_.upper.transitionFrame, 0.1f);
				ImGui::TreePop();
			}
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/BossState.json");
				JSON_OBJECT("StateRoot");
				JSON_SAVE(jsonData_.root.allFrame);
				JSON_SAVE(jsonData_.root.transitionFrame);
				JSON_ROOT();
				JSON_OBJECT("StateTwoHandAttack");
				JSON_SAVE(jsonData_.twoHand.allFrame);
				JSON_SAVE(jsonData_.twoHand.transitionFrame);
				JSON_ROOT();
				JSON_OBJECT("StateUpperAttack");
				JSON_SAVE(jsonData_.upper.allFrame);
				JSON_SAVE(jsonData_.upper.transitionFrame);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
}
// 特定の型に対する GetStateEnum の特殊化
template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateRoot>() {
	return kRoot;
}

template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateTwoHandAttack>() {
	return kTwoHandAttack;
}

template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateUpperAttack>() {
	return kUpperAttack;
}