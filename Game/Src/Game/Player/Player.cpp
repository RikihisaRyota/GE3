#include "Player.h"

#include <numbers>

#include "imgui.h"

#include "Engine/Json/JsonUtils.h"
#include "Engine/Model/ModelManager.h"
#include "Engine/Input/Input.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Collision/CollisionAttribute.h"
#include "Engine/GPUParticleManager/GPUParticleManager.h"
#include "Engine/WinApp/WinApp.h"

Player::Player() {
	playerModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Player/player.gltf");
	animation_.Initialize("Resources/Animation/Player/animation.gltf", playerModelHandle_);
	for (size_t i = 0; i < State::kCount; ++i) {
		animationInfo_[name_.at(i)].handle = animation_.GetAnimationHandle(name_.at(i));
	}
	worldTransform_.Initialize();
	animationTransform_.Initialize();
	playerBulletManager_ = std::make_unique<PlayerBulletManager>();
	playerUI_ = std::make_unique<PlayerUI>();
	playerBulletManager_->SetPlayerUI(playerUI_.get());
	playerHP_ = std::make_unique<PlayerHP>();
	JSON_OPEN("Resources/Data/Player/player.json");
	JSON_OBJECT("playerProperties");
	JSON_LOAD(walkSpeed_);
	JSON_LOAD(shootingWalkSpeed_);
	JSON_LOAD(gravity_);
	JSON_LOAD(knockBack_);
	JSON_ROOT();
	JSON_OBJECT("playerAnimation");
	JSON_LOAD(transitionCycle_);
	for (auto& animationInfo : animationInfo_) {
		std::string name = animationInfo.first;
		JSON_LOAD_BY_NAME(name, animationInfo.second.animationCycle);
	}
	JSON_ROOT();
	JSON_CLOSE();

#pragma region コライダー
	collider_ = std::make_unique<OBBCollider>();
	collider_->SetName("Player");
	auto& mesh = ModelManager::GetInstance()->GetModel(playerModelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::Player);
	collider_->SetCollisionMask(CollisionAttribute::BossBody | CollisionAttribute::BossAttack | CollisionAttribute::GameObject);
	collider_->SetIsActive(true);
#pragma endregion
}

void Player::Initialize() {
	state_ = kRoot;
	playerBulletManager_->Initialize();
	playerHP_->Initialize();
	playerUI_->Initialize();
	worldTransform_.Reset();
	worldTransform_.translate.y = 6.0f;
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	velocity_ = { 0.0f,0.0f,0.0f };
	acceleration_ = { 0.0f,0.0f,0.0f };
	UpdateTransform();
}

void Player::Update(CommandContext& commandContext) {
	velocity_ = { 0.0f,0.0f,0.0f };
	colliderColor_ = { 0.0f,0.0f,1.0f,1.0f };
	preState_ = state_;
	if (tmpState_.has_value()) {
		state_ = tmpState_.value();
		tmpState_ = std::nullopt;
	}
	else {
		switch (state_) {
		case Player::kRoot:
		case Player::kWalk:
		case Player::kShootingWalk:

			state_ = kRoot;
			break;
		case Player::kHitDamage:
			break;
		case Player::kCount:
			break;
		default:
			break;
		}
	}
	switch (state_) {
	case Player::kRoot:
	case Player::kWalk:
	case Player::kShootingWalk:
		Shot();
		Move();
		break;
	case Player::kHitDamage:
		if (!onTransition_) {
			worldTransform_.translate.x = Lerp(knockBackStartPos_.x, knockBackEndPos_.x, animationTime_);
			worldTransform_.translate.z = Lerp(knockBackStartPos_.z, knockBackEndPos_.z, animationTime_);
		}
		break;
	case Player::kCount:
		break;
	default:
		break;
	}
	acceleration_.y -= gravity_;
	acceleration_ *= 0.9f;
	velocity_ += acceleration_;
	worldTransform_.translate += velocity_;
	Vector3 playerForward = Vector3(velocity_.x, 0.0f, velocity_.z);
	if (playerForward.Length() != 0.0f) {
		PlayerRotate(playerForward.Normalized());
	}

	AnimationUpdate(commandContext);

	BulletUpdate();

	UpdateTransform();

	GPUParticleSpawn();

	playerUI_->Update();

}

void Player::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(animationTransform_, animation_, *viewProjection_, playerModelHandle_, commandContext);

	playerBulletManager_->Draw(viewProjection, commandContext);
}

void Player::DrawSprite(CommandContext& commandContext) {
	playerUI_->Draw(commandContext);
}

void Player::DrawDebug(const ViewProjection& viewProjection) {
	animation_.DrawLine(animationTransform_);
	collider_->DrawCollision(viewProjection, colliderColor_);
	playerBulletManager_->DrawDebug(viewProjection);
}

void Player::BulletUpdate() {
	playerBulletManager_->Update();
}

void Player::Shot() {
	auto input = Input::GetInstance();
	if (input->PushKey(DIK_SPACE) || input->PushGamepadButton(Button::RT)) {
		playerBulletManager_->Create(worldTransform_);
	}
}

void Player::OnCollision(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "Boss" ||
		desc.collider->GetName() == "GameObject") {
		// ワールド空間の押し出しベクトル
		Vector3 pushVector = desc.normal * desc.depth;
		auto parent = worldTransform_.parent_;
		if (parent) {
			pushVector = Inverse(parent->rotate) * pushVector;
		}
		// 上から乗ったら
		if (std::fabs(Dot(desc.normal, { 0.0f,-1.0f,0.0f })) >= 0.5f) {
			acceleration_.y = 0.0f;
		}

		worldTransform_.translate += pushVector;

		UpdateTransform();
		colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	}
	if (desc.collider->GetName() == "BossAttack") {
		if (state_ != kHitDamage) {
			velocity_ = { 0.0f,0.0f,0.0f };
			playerHP_->HitDamage(1);
			tmpState_ = kHitDamage;
			Vector3 vector = Vector3(desc.normal.x, 0.0f, desc.normal.z);
			if (vector.Length() == 0.0f) {
				vector.y = 1.0f;
			}
			vector = vector.Normalized();
			knockBackStartPos_ = MakeTranslateMatrix(worldTransform_.matWorld);
			knockBackEndPos_ = MakeTranslateMatrix(worldTransform_.matWorld) + vector * knockBack_;
			worldTransform_.rotate = MakeLookRotation(-vector);
			UpdateTransform();

			colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
		}
	}
}

void Player::GPUParticleSpawn() {
	//for (auto& joint : animation_.skeleton.joints) {
	//	if (!joint.parent.has_value()) {
	//		continue;
	//	}
	//	Matrix4x4 worldMatrix = joint.skeletonSpaceMatrix * worldTransform_.matWorld;
	//	Matrix4x4 parentMatrix = animation_.skeleton.joints.at(*joint.parent).skeletonSpaceMatrix * worldTransform_.matWorld;

	//	Vector3 worldPos = MakeTranslateMatrix(worldMatrix);
	//	Vector3 parentPos = MakeTranslateMatrix(parentMatrix);
	//	Vector3 born = (worldPos - parentPos);

	//	// 0
	//	{
	//		float startScaleMax = 0.25f;
	//		float startScaleMin = 0.125f;
	//		float scaleMax = born.Length() * startScaleMax;
	//		float scaleMin = born.Length() * startScaleMin;
	//		GPUParticleShaderStructs::Emitter emitterForGPU = {
	//	   .emitterArea{
	//			   .sphere{
	//					.radius = born.Length() * 0.5f,
	//				},
	//				.position{worldPos},
	//				.type = 1,
	//		   },

	//	   .scale{
	//		   .range{
	//			   .start{
	//				   .min = {scaleMin,scaleMin,scaleMin},
	//				   .max = {scaleMax,scaleMax,scaleMax},
	//			   },
	//			   .end{
	//				   .min = {0.0f,0.0f,0.0f},
	//				   .max = {0.0f,0.0f,0.0f},
	//			   },
	//		   },
	//	   },

	//	   .rotate{
	//		   .rotate = 0.0f,
	//	   },

	//	   .velocity{
	//		   .range{
	//			   .min = {0.0f,0.0f,0.0f},
	//			   .max = {0.0f,0.0f,0.0f},
	//		   }
	//	   },

	//	   .color{
	//		   .range{
	//			   .start{
	//				   .min = {0.0f,0.8f,0.2f,1.0f},
	//				   .max = {0.0f,1.0f,0.5f,1.0f},
	//			   },
	//			   .end{
	//				   .min = {0.0f,0.6f,0.1f,1.0f},
	//				   .max = {0.0f,0.8f,0.3f,1.0f},
	//			   },
	//		   },
	//	   },

	//	   .frequency{
	//		   .interval = 0,
	//		   .isLoop = false,
	//		   //.lifeTime = 120,
	//	   },

	//	   .particleLifeSpan{
	//		   .range{
	//			   .min = 1,
	//			   .max = 1,
	//		   }
	//	   },

	//	   .textureIndex = 0,

	//	   .createParticleNum = 1 << 10,
	//		};

	//		gpuParticleManager_->CreateParticle(emitterForGPU);
	//	}
	//}
}

void Player::UpdateTransform() {
	worldTransform_.UpdateMatrix();
	auto& mesh = ModelManager::GetInstance()->GetModel(playerModelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
	animationTransform_.UpdateMatrix();
}

void Player::Move() {
	auto input = Input::GetInstance();
#pragma region ゲームパット
	// ゲームパットの状態を得る変数
	XINPUT_STATE joyState{};
	Vector3 vector{};
	// ゲームパットの状況取得
	// 入力がなかったら何もしない
	if (input->IsControllerConnected()) {
		const float kMargin = 0.7f;
		// 移動量
		Vector3 move = {
			input->GetLeftStick().x,
			0.0f,
			input->GetLeftStick().y,
		};
		if (move.Length() > kMargin) {
			vector = move;
		}
	}
#pragma endregion
#pragma region キーボード
	if (input->PushKey(DIK_W)) {
		vector.z = 1.0f;
	}
	if (input->PushKey(DIK_S)) {
		vector.z = -1.0f;
	}
	if (input->PushKey(DIK_A)) {
		vector.x = -1.0f;
	}
	if (input->PushKey(DIK_D)) {
		vector.x = 1.0f;
	}
	// 移動量に速さを反映
	if (vector != Vector3(0.0f, 0.0f, 0.0f)) {
		tmpState_ = kWalk;
		// 回転行列生成前にゼロベクトルかどうかをチェックし、処理をスキップする
			// 回転行列生成
		Matrix4x4 rotate = MakeRotateYMatrix(viewProjection_->rotation_.y);
		// オフセットをカメラの回転に合わせて回転させる
		vector = TransformNormal(vector.Normalized(), rotate);
		if (vector.Length()==0.0f) {
			int a = 1;
			a = 0;
		}
	}
	if (input->PushKey(DIK_LSHIFT) ||
		input->PushGamepadButton(Button::LT)) {
		tmpState_ = kShootingWalk;
		worldTransform_.rotate = Slerp(worldTransform_.rotate, MakeRotateYAngleQuaternion(viewProjection_->rotation_.y), 0.6f);
	}
	float speed = 0.0f;
	switch (state_) {
	case Player::kRoot:
		break;
	case Player::kWalk:
		speed = walkSpeed_;
		break;
	case Player::kShootingWalk:
		speed = shootingWalkSpeed_;
		break;
	case Player::kCount:
		break;
	default:
		break;
	}
	velocity_ = vector * speed;
}

void Player::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Player")) {
		ImGui::DragFloat3("position", &worldTransform_.translate.x, 0.1f);
		ImGui::DragFloat3("velocity", &velocity_.x, 0.1f);
		ImGui::DragFloat3("acceleration", &acceleration_.x, 0.1f);
		auto& material = ModelManager::GetInstance()->GetModel(playerModelHandle_).GetMaterialData();
		ImGui::DragFloat4("color", &material.color.x, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("environmentCoefficient", &material.environmentCoefficient, 0.1f, 0.0f, 1.0f);
		if (ImGui::TreeNode("PlayerProperties")) {
			ImGui::DragFloat("gravity", &gravity_, 0.01f, 0.0f);
			ImGui::DragFloat("knockBack", &knockBack_, 0.1f, 0.0f);
			ImGui::DragFloat("walkSpeed_", &walkSpeed_, 0.1f, 0.0f);
			ImGui::DragFloat("shootingWalkSpeed_", &shootingWalkSpeed_, 0.1f, 0.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Player/player.json");
				JSON_OBJECT("playerProperties");
				JSON_SAVE(gravity_);
				JSON_SAVE(walkSpeed_);
				JSON_SAVE(shootingWalkSpeed_);
				JSON_SAVE(knockBack_);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("PlayerAnimation")) {
			ImGui::DragFloat("transitionCycle_", &transitionCycle_, 1.0f, 0.0f);
			for (auto& animationInfo : animationInfo_) {
				ImGui::DragFloat((animationInfo.first + "Cycle").c_str(), &animationInfo.second.animationCycle, 1.0f, 0.0f);
			}
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Player/player.json");
				JSON_OBJECT("playerAnimation");
				JSON_SAVE(transitionCycle_);
				for (auto& animationInfo : animationInfo_) {
					std::string name = animationInfo.first;
					JSON_SAVE_BY_NAME(name, animationInfo.second.animationCycle);
				}
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	playerBulletManager_->DrawImGui();
	playerHP_->DrawImGui();
}

void Player::AnimationUpdate(CommandContext& commandContext) {
	if (state_ != preState_) {
		onTransition_ = true;
		transitionTime_ = 0.0f;
		currentAnimationHandle_ = animationInfo_[name_.at(state_)].handle;
		preAnimationHandle_ = animationInfo_[name_.at(preState_)].handle;
	}
	if (!onTransition_) {


		switch (state_) {
		case Player::kRoot:
		case Player::kWalk:
		{
			animationTime_ += 1.0f / animationInfo_[name_.at(state_)].animationCycle;
		}
		break;
		break;
		case Player::kShootingWalk:
		{
			Vector3 length = { velocity_.x,0.0f,velocity_.z };
			if (length.Length() != 0.0f) {
				animationTime_ += 1.0f / animationInfo_[name_.at(state_)].animationCycle;
			}
		}
		break;
		case Player::kHitDamage:
		{
			animationTime_ += 1.0f / animationInfo_[name_.at(state_)].animationCycle;
			if (animationTime_ >= 1.0f) {
				tmpState_ = kRoot;
			}
		}
		break;
		case Player::kCount:
			break;
		default:
			break;
		}
		currentAnimationHandle_ = animationInfo_[name_.at(state_)].handle;
		animationTime_ = std::fmodf(animationTime_, 1.0f);
		animation_.Update(currentAnimationHandle_, animationTime_, commandContext, playerModelHandle_);
	}
	else {
		transitionTime_ += 1.0f / transitionCycle_;

		animation_.Update(preAnimationHandle_, animationTime_, currentAnimationHandle_, 0.0f, transitionTime_, commandContext, playerModelHandle_);
		if (transitionTime_ >= 1.0f) {
			onTransition_ = false;
			transitionTime_ = 0.0f;
			animationTime_ = 0.0f;
		}
	}
}

void Player::PlayerRotate(const Vector3& move) {
	Vector3 vector = Conjugation(worldTransform_.rotate) * move.Normalized();
	vector = vector.Normalized();
	if (Dot({ 0.0f,0.0f,1.0f }, vector) < 0.999f) {
		Quaternion diff = MakeFromTwoVector({ 0.0f,0.0f,1.0f }, vector);
		worldTransform_.rotate = Slerp(Quaternion::identity, diff, 0.1f) * worldTransform_.rotate;
	}
}
