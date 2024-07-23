#include "BossStateManager.h"

#include "../Boss.h"
#include "Engine/Json/JsonUtils.h"
#include "Engine/ImGui/ImGuiManager.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Math/OBB.h"
#include "Engine/Collision/CollisionAttribute.h"

void BossStateRoot::Initialize(CommandContext& commandContext) {
	auto boss = manager_.boss_;
	SetDesc();
	//animationHandle_ = manager_.boss_->GetAnimation()->GetAnimationHandle("idle");
	time_ = 0.0f;
	if (manager_.GetPreState() != BossStateManager::State::kRoot) {
		//manager_.gpuParticleManager_->CreateTransformModelParticle(manager_.GetModelHandle(), manager_.GetWorldTransform().matWorld, boss->GetModelHandle(),/* *boss->GetAnimation(),*/ boss->GetWorldMatrix(), data_.transformEmitter, commandContext);
	}
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
		//if (time_ >= 1.0f) {
		//	BossStateManager::State tmp = static_cast<BossStateManager::State>((rnd_.NextUIntLimit() % 1) + int(BossStateManager::State::kCarAttack));
		//	switch (tmp) {
		//	case BossStateManager::State::kCarAttack:
		//		manager_.ChangeState<BossStateCarAttack>();
		//		break;
		//	}
		//}
		time_ = std::fmod(time_, 1.0f);
	}

	auto boss = manager_.boss_;
	auto animation = manager_.boss_->GetAnimation();
	if (inTransition_) {

	}
	else {
		//animation->Update(animationHandle_, time_, commandContext, boss->GetModelHandle());
	}
}
void BossStateRoot::DebugDraw() {}
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

void BossStateCarAttack::Initialize(CommandContext& commandContext) {
	SetDesc();
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/train.gltf");
	railModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/rail.gltf");
	worldTransform_.Initialize();
	railWorldTransform_.Initialize();
	collider_ = std::make_unique<OBBCollider>();
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::Boss);
	collider_->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
	collider_->SetName("Boss");
	collider_->SetIsActive(true);
	time_ = 0.0f;
	SetLocation();
	auto boss = manager_.boss_;
	if (manager_.GetPreState() == BossStateManager::State::kRoot) {
		//manager_.gpuParticleManager_->CreateTransformModelParticle(boss->GetModelHandle(),/* *boss->GetAnimation(), */boss->GetWorldMatrix(), modelHandle_, worldTransform_.matWorld, data_.transformEmitter, commandContext);
		GPUParticleShaderStructs::TransformAreaEmitterForCPU  emitter = data_.transformRailEmitter;
		emitter.emitterArea.position = railWorldTransform_.translate;
		//manager_.gpuParticleManager_->CreateTransformModelAreaParticle(railModelHandle_, railWorldTransform_.matWorld, emitter, commandContext);
	}
	else {
		//manager_.gpuParticleManager_->CreateTransformModelParticle(manager_.GetModelHandle(), boss->GetWorldMatrix(), modelHandle_, worldTransform_.matWorld, data_.transformEmitter, commandContext);
	}
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

	}
	else {
		float t = 0.0f;
		if (time_ < 0.5f) {
			t = 8.0f * time_ * time_ * time_ * time_;
		}
		else {
			t = 1.0f - std::pow(-2.0f * time_ + 2.0f, 4.0f) / 2.0f;
		}
		worldTransform_.translate.x = Lerp(data_.start.x, data_.end.x, t);
		UpdateTransform();
		//manager_.gpuParticleManager_->CreateVertexParticle(modelHandle_, worldTransform_.matWorld, data_.vertexEmitter, commandContext);
		//manager_.gpuParticleManager_->CreateVertexParticle(railModelHandle_, railWorldTransform_.matWorld, data_.vertexEmitter, commandContext);
	}

	if (time_ >= 1.0f && !inTransition_) {
		manager_.ChangeState<BossStateRoot>();
		GPUParticleShaderStructs::VertexEmitterForCPU  emitter = data_.vertexEmitter;
		emitter.velocity.range.min = { -0.5f,-0.1f,-0.5f };
		emitter.velocity.range.max = { +0.5f,+0.1f,+0.5f };
		emitter.particleLifeSpan.range.min = 30;
		emitter.particleLifeSpan.range.max = 60;
		//manager_.gpuParticleManager_->CreateVertexParticle(railModelHandle_, railWorldTransform_.matWorld, emitter, commandContext);
	}
}

void BossStateCarAttack::DebugDraw() {
	collider_->DrawCollision({ 0.0f,1.0f,0.2f,1.0f });
}

void BossStateCarAttack::OnCollision(const ColliderDesc& collisionInfo) {
	collisionInfo;
}

void BossStateCarAttack::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	railWorldTransform_.UpdateMatrix();
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld));
	collider_->SetSize(data_.collider.size);
	collider_->SetOrientation(worldTransform_.rotate);

}

void BossStateCarAttack::SetLocation() {

	float random = rnd_.NextFloatUnit();
	if (random >= 0.5f) {
		attackLocation_ |= AttackLocation::kRight;
	}
	else {
		attackLocation_ |= AttackLocation::kLeft;
	}
	random = rnd_.NextFloatUnit();
	if (random >= 0.5f) {
		attackLocation_ |= AttackLocation::kFront;
	}
	else {
		attackLocation_ |= AttackLocation::kBack;
	}

	Quaternion rightRotate = MakeRotateYAngleQuaternion(DegToRad(-90.0f));
	Quaternion leftRotate = MakeRotateYAngleQuaternion(DegToRad(90.0f));
	Vector3 railPos = Vector3(data_.start.x, data_.start.y * 0.5f, 0.0f);
	switch (attackLocation_) {
		// 右前
	case AttackLocation::kRight | AttackLocation::kFront:
	{
		data_.start.x *= -1.0f;
		data_.end.x *= -1.0f;
		worldTransform_.translate = data_.start + Vector3(0.0f, 0.0f, 1.0f) * data_.frontAndBackOffset;
		worldTransform_.rotate = rightRotate;
		railWorldTransform_.translate = railPos + Vector3(0.0f, 0.0f, 1.0f) * data_.frontAndBackOffset;
		railWorldTransform_.rotate = rightRotate;
	}
	break;
	// 右後ろ
	case AttackLocation::kRight | AttackLocation::kBack:
	{
		data_.start.x *= -1.0f;
		data_.end.x *= -1.0f;
		worldTransform_.translate = data_.start + Vector3(0.0f, 0.0f, 1.0f) * -data_.frontAndBackOffset;
		worldTransform_.rotate = rightRotate;
		railWorldTransform_.translate = railPos + Vector3(0.0f, 0.0f, 1.0f) * -data_.frontAndBackOffset;
		railWorldTransform_.rotate = rightRotate;
	}
	break;
	// 左前
	case AttackLocation::kLeft | AttackLocation::kFront:
	{
		worldTransform_.translate = data_.start + Vector3(0.0f, 0.0f, 1.0f) * data_.frontAndBackOffset;
		worldTransform_.rotate = leftRotate;
		railWorldTransform_.translate = railPos + Vector3(0.0f, 0.0f, 1.0f) * data_.frontAndBackOffset;
		railWorldTransform_.rotate = leftRotate;
	}
	break;
	// 左後ろ
	case AttackLocation::kLeft | AttackLocation::kBack:
	{
		worldTransform_.translate = data_.start + Vector3(0.0f, 0.0f, 1.0f) * -data_.frontAndBackOffset;
		worldTransform_.rotate = leftRotate;
		railWorldTransform_.translate = railPos + Vector3(0.0f, 0.0f, 1.0f) * -data_.frontAndBackOffset;
		railWorldTransform_.rotate = leftRotate;
	}
	break;
	default:
		break;
	}
	UpdateTransform();

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
	JSON_LOAD_BY_NAME("frontAndBackOffset", jsonData_.carAttack.frontAndBackOffset);
	JSON_LOAD_BY_NAME("start", jsonData_.carAttack.start);
	JSON_LOAD_BY_NAME("end", jsonData_.carAttack.end);
	JSON_PARENT();
	JSON_ROOT();

	JSON_CLOSE();
	GPUParticleShaderStructs::Load("root", jsonData_.root.transformEmitter);
	GPUParticleShaderStructs::Load("carAttack", jsonData_.carAttack.vertexEmitter);
	GPUParticleShaderStructs::Load("carAttack", jsonData_.carAttack.transformEmitter);
	GPUParticleShaderStructs::Load("rail", jsonData_.carAttack.transformRailEmitter);

	activeStateEnum_ = kRoot;
	standbyStateEnum_ = kRoot;
	ChangeState<BossStateRoot>();
}

void BossStateManager::Update(CommandContext& commandContext) {
	if (standbyState_) {
		preStateEnum_ = activeStateEnum_;
		activeStateEnum_ = standbyStateEnum_;
		standbyStateEnum_ = kNone;
		activeState_ = std::move(standbyState_);
		activeState_->Initialize(commandContext);
	}

	if (activeState_) {
		activeState_->Update(commandContext);
#ifdef _DEBUG
		activeState_->DebugDraw();
#endif // _DEBUG

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
					ImGui::DragFloat("frontAndBackOffset", &jsonData_.carAttack.frontAndBackOffset, 0.1f, 0.0f);
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
				JSON_SAVE_BY_NAME("frontAndBackOffset", jsonData_.carAttack.frontAndBackOffset);
				JSON_SAVE_BY_NAME("start", jsonData_.carAttack.start);
				JSON_SAVE_BY_NAME("end", jsonData_.carAttack.end);
				JSON_PARENT();
				JSON_ROOT();
				JSON_CLOSE();
				GPUParticleShaderStructs::Save("root", jsonData_.root.transformEmitter);
				GPUParticleShaderStructs::Save("carAttack", jsonData_.carAttack.vertexEmitter);
				GPUParticleShaderStructs::Save("carAttack", jsonData_.carAttack.transformEmitter);
				GPUParticleShaderStructs::Save("rail", jsonData_.carAttack.transformRailEmitter);
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	GPUParticleShaderStructs::Debug("root", jsonData_.root.transformEmitter);
	GPUParticleShaderStructs::Debug("carAttack", jsonData_.carAttack.vertexEmitter);
	GPUParticleShaderStructs::Debug("carAttack", jsonData_.carAttack.transformEmitter);
	GPUParticleShaderStructs::Debug("rail", jsonData_.carAttack.transformRailEmitter);
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