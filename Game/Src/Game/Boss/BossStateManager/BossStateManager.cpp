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
		data_.transformEmitter.startModelWorldMatrix = manager_.GetWorldTransform().matWorld;
		data_.transformEmitter.endModelWorldMatrix = boss->GetWorldMatrix();

		switch (manager_.GetPreState()) {
		case  BossStateManager::State::kRushAttack:
		{
			data_.transformEmitter.color.range.start = manager_.jsonData_.rushAttack.trainEmitter.color.range.end;
			data_.transformEmitter.color.range.end = boss->GetVertexEmitter().color.range.start;
		}
		break;
		default:
			break;
		}
		manager_.gpuParticleManager_->SetTransformModelEmitter(manager_.GetModelHandle(), boss->GetModelHandle(), data_.transformEmitter);
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
		if (time_ >= 1.0f) {
			BossStateManager::State tmp = static_cast<BossStateManager::State>((rnd_.NextUIntLimit() % 1) + int(BossStateManager::State::kRushAttack));
			switch (tmp) {
			case BossStateManager::State::kRushAttack:
				manager_.ChangeState<BossStateRushAttack >();
				break;
			}
		}
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

void BossStateRushAttack::Initialize(CommandContext& commandContext) {
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

	// VertexEmitter
	data_.trainEmitter.isAlive = false;
	data_.railEmitter.isAlive = false;
	// TransformEmitter
	data_.transformTrainEmitter.color.range.start = boss->GetVertexEmitter().color.range.start;
	data_.transformRailEmitter.color.range.start = boss->GetVertexEmitter().color.range.start;
	if (manager_.GetPreState() == BossStateManager::State::kRoot) {
		data_.transformTrainEmitter.startModelWorldMatrix = boss->GetWorldMatrix();
		data_.transformTrainEmitter.endModelWorldMatrix = worldTransform_.matWorld;
		manager_.gpuParticleManager_->SetTransformModelEmitter(boss->GetModelHandle(), modelHandle_, data_.transformTrainEmitter);
		data_.transformRailEmitter.modelWorldMatrix = railWorldTransform_.matWorld;
		GPUParticleShaderStructs::TransformAreaEmitterForCPU  emitter = data_.transformRailEmitter;
		emitter.emitterArea.position = worldTransform_.translate;
		emitter.modelWorldMatrix = railWorldTransform_.matWorld;
		manager_.gpuParticleManager_->SetTransformAreaEmitter(railModelHandle_, emitter);
	}
}

void BossStateRushAttack::SetDesc() {
	data_ = manager_.jsonData_.rushAttack;
}

void BossStateRushAttack::Update(CommandContext& commandContext) {
	auto boss = manager_.boss_;

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
		if (time_ >= 1.0f) {
			data_.trainEmitter.isAlive = true;
			data_.railEmitter.isAlive = true;
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
		data_.trainEmitter.localTransform.translate = MakeTranslateMatrix(worldTransform_.matWorld);
		data_.trainEmitter.localTransform.rotate = Inverse(MakeRotateMatrix(worldTransform_.matWorld));
		data_.railEmitter.localTransform.translate = MakeTranslateMatrix(railWorldTransform_.matWorld);
		data_.railEmitter.localTransform.rotate = Inverse(MakeRotateMatrix(railWorldTransform_.matWorld));
	}

	if (time_ >= 1.0f && !inTransition_) {
		manager_.ChangeState<BossStateRoot>();
		data_.trainEmitter.isAlive = false;
		data_.railEmitter.isAlive = false;
		data_.transformRailVertexEmitter.color = data_.railEmitter.color;
		data_.transformRailVertexEmitter.scale.range.start = data_.railEmitter.scale.range.start;
		data_.transformRailVertexEmitter.localTransform.translate = data_.railEmitter.localTransform.translate;
		data_.transformRailVertexEmitter.localTransform.rotate = data_.railEmitter.localTransform.rotate;

		manager_.gpuParticleManager_->SetVertexEmitter(railModelHandle_, data_.transformRailVertexEmitter);
	}
	manager_.gpuParticleManager_->SetVertexEmitter(modelHandle_, data_.trainEmitter);
	manager_.gpuParticleManager_->SetVertexEmitter(railModelHandle_, data_.railEmitter);
}

void BossStateRushAttack::DebugDraw() {
	collider_->DrawCollision({ 0.0f,1.0f,0.2f,1.0f });
}

void BossStateRushAttack::OnCollision(const ColliderDesc& collisionInfo) {
	collisionInfo;
}

void BossStateRushAttack::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	railWorldTransform_.UpdateMatrix();
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld));
	collider_->SetSize(data_.collider.size);
	collider_->SetOrientation(worldTransform_.rotate);

}

void BossStateRushAttack::SetLocation() {

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
	Vector3 railPos = Vector3(0.0f, data_.start.y * 0.5f, 0.0f);
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

void BossStateSmashAttack::Initialize(CommandContext& commandContext) {
	SetDesc();
	modelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Boss/hand.gltf");
	worldTransform_.Initialize();
	worldTransform_.translate.y = data_.y;
	worldTransform_.UpdateMatrix();
	for (uint32_t i = 0; i < data_.smashCount; i++) {
		SmashDesc desc{};
		desc.collider = std::make_unique<OBBCollider>();
		desc.collider->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
		desc.collider->SetCollisionAttribute(CollisionAttribute::Boss);
		desc.collider->SetCollisionMask(CollisionAttribute::Player | CollisionAttribute::PlayerBullet);
		desc.collider->SetName("Boss");
		desc.collider->SetIsActive(false);
		desc.transform.Initialize();
		desc.transform.parent_ = &worldTransform_;
		desc.transform.UpdateMatrix();
		// カウント
		uint32_t emitterCount = desc.emitter.emitterCount;
		desc.emitter = data_.smashEmitter;
		desc.emitter.emitterCount = emitterCount;
		desc.start = Vector3(0.0f, 10.0f, 0.0f);
		desc.end = Vector3(0.0f, 0.0f, 0.0f);
		smash_.emplace_back(std::move(desc));
	}
	time_ = 0.0f;
	auto boss = manager_.boss_;

	// VertexEmitter
	for (auto& smash : smash_) {
		smash.emitter.isAlive = false;
	}
	// TransformEmitter
	data_.smashEmitter.color.range.start = boss->GetVertexEmitter().color.range.start;
	if (manager_.GetPreState() == BossStateManager::State::kRoot) {
		for (auto& smash : smash_) {
			data_.transformEmitter.startModelWorldMatrix = boss->GetWorldMatrix();
			data_.transformEmitter.endModelWorldMatrix = smash.transform.matWorld;
			manager_.gpuParticleManager_->SetTransformModelEmitter(boss->GetModelHandle(), modelHandle_, data_.transformEmitter);
		}
	}
}

void BossStateSmashAttack::SetDesc() {
	data_ = manager_.jsonData_.smashAttack;
}

void BossStateSmashAttack::Update(CommandContext& commandContext) {
	auto boss = manager_.boss_;

	if (inTransition_) {
		time_ += 1.0f / data_.transitionFrame;
		if (time_ >= 1.0f) {
			for (auto& smash : smash_) {
				smash.emitter.isAlive = true;
			}
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
		for (auto& smash : smash_) {
			float t = 0.0f;
			if (time_ < 0.5f) {
				t = 8.0f * time_ * time_ * time_ * time_;
			}
			else {
				t = 1.0f - std::pow(-2.0f * time_ + 2.0f, 4.0f) / 2.0f;
			}
			smash.transform.translate = Lerp(smash.start, smash.end, t);
			UpdateTransform();

			smash.emitter.localTransform.translate = MakeTranslateMatrix(smash.transform.matWorld);
			smash.emitter.localTransform.rotate = Inverse(MakeRotateMatrix(smash.transform.matWorld));
		}
	}

	if (time_ >= 1.0f && !inTransition_) {
		for (auto& smash : smash_) {
			smash.emitter.isAlive = false;
		}
		manager_.ChangeState<BossStateRoot>();

	}
	for (auto& smash : smash_) {
		manager_.gpuParticleManager_->SetVertexEmitter(modelHandle_, smash.emitter);
	}
}

void BossStateSmashAttack::DebugDraw() {}

void BossStateSmashAttack::OnCollision(const ColliderDesc& collisionInfo) {}

void BossStateSmashAttack::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	for (auto& smash : smash_) {
		smash.transform.UpdateMatrix();
		smash.collider->SetCenter(smash.transform.translate);
		smash.collider->SetOrientation(smash.transform.rotate);
		smash.collider->SetSize(smash.transform.scale);
	}
}

void BossStateSmashAttack::SetLocation() {
	for (auto& smash : smash_) {

	}
}

void BossStateManager::Initialize() {
	JSON_OPEN("Resources/Data/Boss/bossState.json");
	JSON_OBJECT("Root");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.root.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.root.transitionFrame);
	JSON_ROOT();

	JSON_OBJECT("RushAttack");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.rushAttack.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.rushAttack.transitionFrame);
	JSON_PARENT();
	JSON_OBJECT("Collider");
	JSON_LOAD_BY_NAME("size", jsonData_.rushAttack.collider.size);
	JSON_PARENT();
	JSON_OBJECT("Properties");
	JSON_LOAD_BY_NAME("frontAndBackOffset", jsonData_.rushAttack.frontAndBackOffset);
	JSON_LOAD_BY_NAME("start", jsonData_.rushAttack.start);
	JSON_LOAD_BY_NAME("end", jsonData_.rushAttack.end);
	JSON_PARENT();
	JSON_ROOT();

	JSON_OBJECT("SmashAttack");
	JSON_OBJECT("Animation");
	JSON_LOAD_BY_NAME("allFrame", jsonData_.smashAttack.allFrame);
	JSON_LOAD_BY_NAME("transitionFrame", jsonData_.smashAttack.transitionFrame);
	JSON_PARENT();
	JSON_OBJECT("Collider");
	JSON_LOAD_BY_NAME("size", jsonData_.smashAttack.collider.size);
	JSON_PARENT();
	JSON_OBJECT("Properties");
	JSON_LOAD_BY_NAME("height", jsonData_.smashAttack.y);
	JSON_LOAD_BY_NAME("count", jsonData_.smashAttack.smashCount);
	JSON_PARENT();
	JSON_ROOT();

	JSON_CLOSE();
	GPUParticleShaderStructs::Load("root", jsonData_.root.transformEmitter);

	GPUParticleShaderStructs::Load("train", jsonData_.rushAttack.trainEmitter);
	GPUParticleShaderStructs::Load("train", jsonData_.rushAttack.transformTrainEmitter);
	GPUParticleShaderStructs::Load("rail", jsonData_.rushAttack.railEmitter);
	GPUParticleShaderStructs::Load("transformRailVertexEmitter", jsonData_.rushAttack.transformRailVertexEmitter);
	GPUParticleShaderStructs::Load("rail", jsonData_.rushAttack.transformRailEmitter);
	GPUParticleShaderStructs::Load("smash", jsonData_.smashAttack.smashEmitter);
	GPUParticleShaderStructs::Load("smash", jsonData_.smashAttack.transformEmitter);
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
				case kRushAttack:
					ChangeState<BossStateRushAttack>();
					break;
				case kSmashAttack:
					ChangeState<BossStateSmashAttack>();
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
			if (ImGui::TreeNode("RushAttack")) {
				if (ImGui::TreeNode("Properties")) {
					ImGui::DragFloat("frontAndBackOffset", &jsonData_.rushAttack.frontAndBackOffset, 0.1f, 0.0f);
					ImGui::DragFloat3("start", &jsonData_.rushAttack.end.x, 0.1f, 0.0f);
					ImGui::DragFloat3("end", &jsonData_.rushAttack.start.x, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Collider")) {
					ImGui::DragFloat3("size", &jsonData_.rushAttack.collider.size.x, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Animation")) {
					ImGui::DragFloat("allFrame", &jsonData_.rushAttack.allFrame, 0.1f, 0.0f);
					ImGui::DragFloat("transitionFrame", &jsonData_.rushAttack.transitionFrame, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("SmashAttack")) {
				if (ImGui::TreeNode("Properties")) {
					ImGui::DragFloat("height", &jsonData_.smashAttack.y, 0.1f, 0.0f);
					int i = jsonData_.smashAttack.smashCount;
					ImGui::DragInt("count", &i, 1, 0, 10);
					jsonData_.smashAttack.smashCount = i;
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Collider")) {
					ImGui::DragFloat3("size", &jsonData_.smashAttack.collider.size.x, 0.1f, 0.0f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Animation")) {
					ImGui::DragFloat("allFrame", &jsonData_.smashAttack.allFrame, 0.1f, 0.0f);
					ImGui::DragFloat("transitionFrame", &jsonData_.smashAttack.transitionFrame, 0.1f, 0.0f);
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

				JSON_OBJECT("RushAttack");
				JSON_OBJECT("Animation");
				JSON_SAVE_BY_NAME("allFrame", jsonData_.rushAttack.allFrame);
				JSON_SAVE_BY_NAME("transitionFrame", jsonData_.rushAttack.transitionFrame);
				JSON_PARENT();
				JSON_OBJECT("Collider");
				JSON_SAVE_BY_NAME("size", jsonData_.rushAttack.collider.size);
				JSON_PARENT();
				JSON_OBJECT("Properties");
				JSON_SAVE_BY_NAME("frontAndBackOffset", jsonData_.rushAttack.frontAndBackOffset);
				JSON_SAVE_BY_NAME("start", jsonData_.rushAttack.start);
				JSON_SAVE_BY_NAME("end", jsonData_.rushAttack.end);
				JSON_PARENT();
				JSON_ROOT();

				JSON_OBJECT("SmashAttack");
				JSON_OBJECT("Animation");
				JSON_SAVE_BY_NAME("allFrame", jsonData_.smashAttack.allFrame);
				JSON_SAVE_BY_NAME("transitionFrame", jsonData_.smashAttack.transitionFrame);
				JSON_PARENT();
				JSON_OBJECT("Collider");
				JSON_SAVE_BY_NAME("size", jsonData_.smashAttack.collider.size);
				JSON_PARENT();
				JSON_OBJECT("Properties");
				JSON_SAVE_BY_NAME("height", jsonData_.smashAttack.y);
				JSON_SAVE_BY_NAME("count", jsonData_.smashAttack.smashCount);
				JSON_PARENT();
				JSON_ROOT();

				JSON_CLOSE();
				//GPUParticleShaderStructs::Save("root", jsonData_.root.transformEmitter);
				//GPUParticleShaderStructs::Save("rushAttack", jsonData_.rushAttack.trainEmitter);
				//GPUParticleShaderStructs::Save("rushAttack", jsonData_.rushAttack.transformTrainEmitter);
				//GPUParticleShaderStructs::Save("rail", jsonData_.rushAttack.railEmitter);
				//GPUParticleShaderStructs::Save("rail", jsonData_.rushAttack.transformRailEmitter);
				//GPUParticleShaderStructs::Save("smash", jsonData_.smashAttack.smashEmitter);
				//GPUParticleShaderStructs::Save("smash", jsonData_.smashAttack.transformEmitter);
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	GPUParticleShaderStructs::Debug("root", jsonData_.root.transformEmitter);
	GPUParticleShaderStructs::Debug("train", jsonData_.rushAttack.trainEmitter);
	GPUParticleShaderStructs::Debug("train", jsonData_.rushAttack.transformTrainEmitter);
	GPUParticleShaderStructs::Debug("rail", jsonData_.rushAttack.railEmitter);
	GPUParticleShaderStructs::Debug("rail", jsonData_.rushAttack.transformRailEmitter);
	GPUParticleShaderStructs::Debug("transformRailVertexEmitter", jsonData_.rushAttack.transformRailVertexEmitter);
	GPUParticleShaderStructs::Debug("smash", jsonData_.smashAttack.smashEmitter);
	GPUParticleShaderStructs::Debug("smash", jsonData_.smashAttack.transformEmitter);

#endif // _DEBUG
}
// 特定の型に対する GetStateEnum の特殊化
template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateRoot>() {
	return kRoot;
}

template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateRushAttack>() {
	return kRushAttack;
}

template<>
BossStateManager::State BossStateManager::GetStateEnum<BossStateSmashAttack>() {
	return kSmashAttack;
}