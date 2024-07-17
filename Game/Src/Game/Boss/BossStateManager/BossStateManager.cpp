#include "BossStateManager.h"

#include "../Boss.h"
#include "Engine/Json/JsonUtils.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Math/OBB.h"
#include "Engine/Collision/CollisionAttribute.h"

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
		/*if (time_ >= 1.0f) {
			BossStateManager::State tmp = static_cast<BossStateManager::State>((rnd_.NextUIntLimit() % 2)+ int(BossStateManager::State::kTwoHandAttack));
			switch (tmp) {
			case BossStateManager::State::kTwoHandAttack:
				manager_.ChangeState<BossStateTwoHandAttack>();
				break;
			case BossStateManager::State::kUpperAttack:
				manager_.ChangeState<BossStateUpperAttack>();
				break;
			}
		}*/
		time_ = std::fmod(time_, 1.0f);
	}

	auto boss = manager_.boss_;
	auto animation = manager_.boss_->GetAnimation();
	if (inTransition_) {
		manager_.gpuParticleManager_->CreateTransformModelParticle(manager_.GetModelHandle(), manager_.GetWorldTransform().matWorld, boss->GetModelHandle(), *boss->GetAnimation(), boss->GetWorldMatrix(), time_, boss->GetVertexEmitter(), commandContext);
	}
	else {
		animation->Update(animationHandle_, time_, commandContext, boss->GetModelHandle());
	}
}
//
//void BossStateTwoHandAttack::Initialize() {
//	SetDesc();
//	animationHandle_ = manager_.boss_->GetAnimation()->GetAnimationHandle("twoHandAttack");
//	time_ = 0.0f;
//	manager_.SetAttackColliderActive(BossStateManager::kTwoHandAttack, true);
//	manager_.SetBodyColliderActive(BossStateManager::kTwoHandAttack, false);
//	manager_.SetColliderColor(BossStateManager::kTwoHandAttack, manager_.boss_->GetAttackColor());
//}
//
//void BossStateTwoHandAttack::SetDesc() {
//	data_ = manager_.jsonData_.twoHand;
//}
//
//void BossStateTwoHandAttack::Update(CommandContext& commandContext) {
//	auto boss = manager_.boss_;
//	auto animation = boss->GetAnimation();
//
//	if (inTransition_) {
//		time_ += 1.0f / data_.transitionFrame;
//		if (time_ >= 1.0f) {
//			inTransition_ = false;
//			time_ = 0.0f;
//		}
//	}
//
//	if (!inTransition_) {
//		time_ += 1.0f / data_.allFrame;
//	}
//
//	time_ = std::clamp(time_, 0.0f, 1.0f);
//
//	if (inTransition_) {
//		animation->Update(manager_.GetAnimationHandle(), manager_.GetAnimationTime(), animationHandle_, 0.0f, time_, commandContext, boss->GetModelHandle());
//	}
//	else {
//		animation->Update(animationHandle_, time_, commandContext, boss->GetModelHandle());
//	}
//
//	if (time_ >= 1.0f && !inTransition_) {
//		manager_.ChangeState<BossStateRoot>();
//		manager_.SetAttackColliderActive(BossStateManager::kTwoHandAttack, false);
//		manager_.SetBodyColliderActive(BossStateManager::kTwoHandAttack, true);
//		manager_.SetColliderColor(BossStateManager::kTwoHandAttack, manager_.boss_->GetDefaultColor());
//	}
//}

void BossStateCarAttack::Initialize() {
	SetDesc();
	//animationHandle_ = manager_.boss_->GetAnimation()->GetAnimationHandle("twoHandAttack");
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/truck.gltf");
	worldTransform_.Initialize();
	worldTransform_.translate = data_.start;
	worldTransform_.UpdateMatrix();
	collider_ = std::make_unique<OBBCollider>();
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::Boss);
	collider_->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
	collider_->SetIsActive(true);
	time_ = 0.0f;
}

void BossStateCarAttack::SetDesc() {
	data_ = manager_.jsonData_.carAttack;
}

void BossStateCarAttack::Update(CommandContext& commandContext) {
	auto boss = manager_.boss_;

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
		if (manager_.GetPreState() == BossStateManager::State::kRoot) {
			manager_.gpuParticleManager_->CreateTransformModelParticle(boss->GetModelHandle(), *boss->GetAnimation(), boss->GetWorldMatrix(), modelHandle_, worldTransform_.matWorld, time_, data_.vertexEmitter, commandContext);
		}
		else {
			manager_.gpuParticleManager_->CreateTransformModelParticle(manager_.GetModelHandle(), boss->GetWorldMatrix(), modelHandle_, worldTransform_.matWorld, time_, data_.vertexEmitter, commandContext);
		}
	}
	else {
		worldTransform_.translate = Lerp(data_.start, data_.end, time_);
		UpdateTransform();
		manager_.gpuParticleManager_->CreateVertexParticle(modelHandle_, worldTransform_.matWorld, data_.vertexEmitter, commandContext);
	}

	if (time_ >= 1.0f && !inTransition_) {
		manager_.ChangeState<BossStateRoot>();
	}
}

void BossStateCarAttack::OnCollision(const ColliderDesc& collisionInfo) {
	collisionInfo;
}

void BossStateCarAttack::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld));
	collider_->SetSize(data_.collider.size);
	collider_->SetOrientation(worldTransform_.rotate);

}


void BossStateManager::Initialize() {
	JSON_OPEN("Resources/Data/Boss/bossState.json");
	JSON_OBJECT("Root");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.root.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.root.transitionFrame);
	JSON_ROOT();

	JSON_OBJECT("CarAttack");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.carAttack.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.carAttack.transitionFrame);
	JSON_PARENT();
	JSON_OBJECT("Collider");
	JSON_LOAD_BY_NAME("size", jsonData_.carAttack.collider.size);
	JSON_PARENT();
	JSON_OBJECT("Properties");
	JSON_LOAD_BY_NAME("start", jsonData_.carAttack.start);
	JSON_LOAD_BY_NAME("end", jsonData_.carAttack.end);
	JSON_PARENT();
	JSON_ROOT();
	JSON_CLOSE();
	GPUParticleShaderStructs::Load("carAttack", jsonData_.carAttack.vertexEmitter);

	activeStateEnum_ = kRoot;
	standbyStateEnum_ = kRoot;
	ChangeState<BossStateRoot>();
}

void BossStateManager::Update(CommandContext& commandContext) {
	if (standbyState_) {
		activeState_ = std::move(standbyState_);
		activeState_->Initialize();
		preStateEnum_ = activeStateEnum_;
		activeStateEnum_ = standbyStateEnum_;
		standbyStateEnum_ = kNone;
	}

	if (activeState_) {
		activeState_->Update(commandContext);
	}
}

void BossStateManager::DrawImGui() {
#ifdef _DEBUG
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Boss")) {
		if (ImGui::TreeNode("BossStateManager")) {
			// ステートを変更するImGui::Comboの作成
			// コンボボックスに渡すための const char* 配列を生成
			std::vector<const char*> stateNamesCStr;
			for (const auto& state : stateNames_) {
				stateNamesCStr.push_back(state.c_str());
			}
			int currentState = static_cast<int>(activeStateEnum_);

			// ステートを変更するImGui::Comboの作成
			if (ImGui::Combo("Change State", &currentState, stateNamesCStr.data(), int(stateNamesCStr.size()))) {
				switch (currentState) {
				case kRoot:
					ChangeState<BossStateRoot>();
					break;
				case kCarAttack:
					ChangeState<BossStateCarAttack>();
					break;
				default:
					break;
				}
			}
			if (ImGui::TreeNode("Root")) {
				if (ImGui::TreeNode("Animation")) {
					ImGui::DragFloat("allFrame", &jsonData_.root.allFrame, 0.1f);
					ImGui::DragFloat("transitionFrame", &jsonData_.root.transitionFrame, 0.1f);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("CarAttack")) {
				if (ImGui::TreeNode("Properties")) {
					ImGui::DragFloat3("start", &jsonData_.carAttack.end.x, 0.1f, 0.0f);
					ImGui::DragFloat3("end", &jsonData_.carAttack.start.x, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Collider")) {
					ImGui::DragFloat3("size", &jsonData_.carAttack.collider.size.x, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Animation")) {
					ImGui::DragFloat("allFrame", &jsonData_.carAttack.allFrame, 0.1f, 0.0f);
					ImGui::DragFloat("transitionFrame", &jsonData_.carAttack.transitionFrame, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}

			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Boss/bossState.json");
				JSON_OBJECT("Root");
				JSON_OBJECT("Animation");
				JSON_SAVE_BY_NAME("allFrame", jsonData_.root.allFrame);
				JSON_SAVE_BY_NAME("transitionFrame", jsonData_.root.transitionFrame);
				JSON_ROOT();

				JSON_OBJECT("CarAttack");
				JSON_OBJECT("Animation");
				JSON_SAVE_BY_NAME("allFrame", jsonData_.carAttack.allFrame);
				JSON_SAVE_BY_NAME("transitionFrame", jsonData_.carAttack.transitionFrame);
				JSON_PARENT();
				JSON_OBJECT("Collider");
				JSON_SAVE_BY_NAME("size", jsonData_.carAttack.collider.size);
				JSON_PARENT();
				JSON_OBJECT("Properties");
				JSON_SAVE_BY_NAME("start", jsonData_.carAttack.start);
				JSON_SAVE_BY_NAME("end", jsonData_.carAttack.end);
				JSON_PARENT();
				JSON_ROOT();
				JSON_CLOSE();
				GPUParticleShaderStructs::Load("carAttack", jsonData_.carAttack.vertexEmitter);
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	GPUParticleShaderStructs::Debug("carAttack", jsonData_.carAttack.vertexEmitter);
#endif // _DEBUG
}
// 特定の型に対する GetStateEnum の特殊化
template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateRoot>() {
	return kRoot;
}

template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateCarAttack>() {
	return kCarAttack;
}