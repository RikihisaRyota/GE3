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
	walkHandle_ = animation_.GetAnimationHandle("walk");
	shootWalkHandle_ = animation_.GetAnimationHandle("shootingWalk");
	idleHandle_ = animation_.GetAnimationHandle("idle");
	worldTransform_.Initialize();
	animationTransform_.Initialize();
	playerBulletManager_ = std::make_unique<PlayerBulletManager>();
	playerUI_ = std::make_unique<PlayerUI>();
	playerBulletManager_->SetPlayerUI(playerUI_.get());
	JSON_OPEN("Resources/Data/Player/player.json");
	JSON_OBJECT("playerProperties");
	JSON_LOAD(walkSpeed_);
	JSON_LOAD(shootingWalkSpeed_);
	JSON_ROOT();
	JSON_OBJECT("playerAnimation");
	JSON_LOAD(transitionCycle_);
	JSON_LOAD(idleAnimationCycle_);
	JSON_LOAD(walkAnimationCycle_);
	JSON_LOAD(shootWalkAnimationCycle_);
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
	collider_->SetCollisionMask(CollisionAttribute::BossBody| CollisionAttribute::BossAttack| CollisionAttribute::GameObject);
	collider_->SetIsActive(true);
#pragma endregion
}

void Player::Initialize() {
	state_ = kRoot;
	playerBulletManager_->Initialize();
	playerUI_->Initialize();
	worldTransform_.Reset();
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	UpdateTransform();
}

void Player::Update(CommandContext& commandContext) {
	colliderColor_ = { 0.0f,0.0f,1.0f,1.0f };
	preState_ = state_;
	state_ = kRoot;

	Move();

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
	Shot();
	playerBulletManager_->Update();
}

void Player::Shot() {
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
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
		worldTransform_.translate += pushVector;

		UpdateTransform();
		colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	}
}

void Player::GPUParticleSpawn() {
	for (auto& joint : animation_.skeleton.joints) {
		if (!joint.parent.has_value()) {
			continue;
		}
		Matrix4x4 worldMatrix = joint.skeletonSpaceMatrix * worldTransform_.matWorld;
		Matrix4x4 parentMatrix = animation_.skeleton.joints.at(*joint.parent).skeletonSpaceMatrix * worldTransform_.matWorld;

		Vector3 worldPos = MakeTranslateMatrix(worldMatrix);
		Vector3 parentPos = MakeTranslateMatrix(parentMatrix);
		Vector3 born = (worldPos - parentPos);

		// 0
		{
			float startScaleMax = 0.25f;
			float startScaleMin = 0.125f;
			float scaleMax = born.Length() * startScaleMax;
			float scaleMin = born.Length() * startScaleMin;
			GPUParticleShaderStructs::Emitter emitterForGPU = {
		   .emitterArea{
				   .sphere{
						.radius = born.Length() * 0.5f,
					},
					.position{worldPos},
					.type = 1,
			   },

		   .scale{
			   .range{
				   .start{
					   .min = {scaleMin,scaleMin,scaleMin},
					   .max = {scaleMax,scaleMax,scaleMax},
				   },
				   .end{
					   .min = {0.0f,0.0f,0.0f},
					   .max = {0.0f,0.0f,0.0f},
				   },
			   },
		   },

		   .rotate{
			   .rotate = 0.0f,
		   },

		   .velocity{
			   .range{
				   .min = {0.0f,0.0f,0.0f},
				   .max = {0.0f,0.0f,0.0f},
			   }
		   },

		   .color{
			   .range{
				   .start{
					   .min = {0.0f,0.8f,0.2f,1.0f},
					   .max = {0.0f,1.0f,0.5f,1.0f},
				   },
				   .end{
					   .min = {0.0f,0.6f,0.1f,1.0f},
					   .max = {0.0f,0.8f,0.3f,1.0f},
				   },
			   },
		   },

		   .frequency{
			   .interval = 0,
			   .isLoop = false,
			   //.lifeTime = 120,
		   },

		   .particleLifeSpan{
			   .range{
				   .min = 1,
				   .max = 1,
			   }
		   },

		   .textureIndex = 0,

		   .createParticleNum = 1 << 10,
			};

			gpuParticleManager_->CreateParticle(emitterForGPU);
		}
	}
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
#pragma region ゲームパット
	// ゲームパットの状態を得る変数
	XINPUT_STATE joyState{};
	Vector3 vector{};
	// ゲームパットの状況取得
	// 入力がなかったら何もしない
	if (Input::GetInstance()->IsControllerConnected()) {
		Input::GetInstance()->GetJoystickState(0, joyState);
		const float kMargin = 0.7f;
		// 移動量
		Vector3 move = {
			static_cast<float>(joyState.Gamepad.sThumbLX),
			0.0f,
			static_cast<float>(joyState.Gamepad.sThumbLY),
		};
		if (move.Length() > kMargin) {
			vector = {
				static_cast<float>(joyState.Gamepad.sThumbLX),
				0.0f,
				static_cast<float>(joyState.Gamepad.sThumbLY),
			};
		}
		vector = move.Normalized();
	}
#pragma endregion
#pragma region キーボード
	if (Input::GetInstance()->PushKey(DIK_W)) {
		vector.z = 1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_S)) {
		vector.z = -1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_A)) {
		vector.x = -1.0f;
	}
	if (Input::GetInstance()->PushKey(DIK_D)) {
		vector.x = 1.0f;
	}
	// 移動量に速さを反映
	if (vector != Vector3(0.0f, 0.0f, 0.0f)) {
		state_ = kWalk;
		// 回転行列生成
		Matrix4x4 rotate = MakeRotateYMatrix(viewProjection_->rotation_.y);
		// オフセットをカメラの回転に合わせて回転させる
		vector = TransformNormal(vector.Normalized(), rotate);
		
		PlayerRotate(vector.Normalized());
	}
	if (Input::GetInstance()->PushKey(DIK_LSHIFT)) {
		state_ = kShootWalk;

		worldTransform_.rotate = Slerp(worldTransform_.rotate, MakeRotateYAngleQuaternion(viewProjection_->rotation_.y), 0.5f);
	}
	float speed = 0.0f;
	switch (state_) {
	case Player::kRoot:
		break;
	case Player::kWalk:
		speed = walkSpeed_;
		break;
	case Player::kShootWalk:
		speed = shootingWalkSpeed_;
		break;
	case Player::kCount:
		break;
	default:
		break;
	}
	worldTransform_.translate += vector * speed;
}

void Player::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Player")) {
		ImGui::DragFloat3("position", &worldTransform_.translate.x, 0.1f);
		auto& material = ModelManager::GetInstance()->GetModel(playerModelHandle_).GetMaterialData();
		ImGui::DragFloat4("color", &material.color.x, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("environmentCoefficient", &material.environmentCoefficient, 0.1f, 0.0f, 1.0f);
		if (ImGui::TreeNode("PlayerProperties")) {
			ImGui::DragFloat("walkSpeed_", &walkSpeed_, 0.1f, 0.0f);
			ImGui::DragFloat("shootingWalkSpeed_", &shootingWalkSpeed_, 0.1f, 0.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Player/player.json");
				JSON_OBJECT("playerProperties");
				JSON_SAVE(walkSpeed_);
				JSON_SAVE(shootingWalkSpeed_);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("PlayerAnimation")) {
			ImGui::DragFloat("transitionCycle_", &transitionCycle_, 1.0f, 0.0f);
			ImGui::DragFloat("idleAnimationCycle_", &idleAnimationCycle_, 1.0f, 0.0f);
			ImGui::DragFloat("walkAnimationCycle_", &walkAnimationCycle_, 1.0f, 0.0f);
			ImGui::DragFloat("shootWalkAnimationCycle_", &shootWalkAnimationCycle_, 1.0f, 0.0f);
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Player/player.json");
				JSON_OBJECT("playerAnimation");
				JSON_SAVE(idleAnimationCycle_);
				JSON_SAVE(walkAnimationCycle_);
				JSON_SAVE(shootWalkAnimationCycle_);
				JSON_SAVE(transitionCycle_);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
	playerBulletManager_->DrawImGui();
}

void Player::AnimationUpdate(CommandContext& commandContext) {
	if (state_ != preState_) {
		onTransition_ = true;
		transitionTime_ = 0.0f;
		switch (state_) {
		case Player::kRoot:
			currentAnimationHandle_ = idleHandle_;
			break;
		case Player::kWalk:
			currentAnimationHandle_ = walkHandle_;
			break;
		case Player::kShootWalk:
			currentAnimationHandle_ = shootWalkHandle_;
			break;
		case Player::kCount:
			break;
		default:
			break;
		}
		switch (preState_) {
		case Player::kRoot:
			preAnimationHandle_ = idleHandle_;
			break;
		case Player::kWalk:
			preAnimationHandle_ = walkHandle_;
			break;
		case Player::kShootWalk:
			preAnimationHandle_ = shootWalkHandle_;
			break;
		case Player::kCount:
			break;
		default:
			break;
		}
	}
	if (!onTransition_) {
		switch (state_) {
		case Player::kRoot:
			currentAnimationHandle_ = idleHandle_;
			animationTime_ += 1.0f / idleAnimationCycle_;
			animationTime_ = std::fmodf(animationTime_, 1.0f);
			break;
		case Player::kWalk:
			currentAnimationHandle_ = walkHandle_;
			animationTime_ += 1.0f / walkAnimationCycle_;
			animationTime_ = std::fmodf(animationTime_, 1.0f);
			break;
		case Player::kShootWalk:
			currentAnimationHandle_ = shootWalkHandle_;
			animationTime_ += 1.0f / shootWalkAnimationCycle_;
			animationTime_ = std::fmodf(animationTime_, 1.0f);
			break;
		case Player::kCount:
			break;
		default:
			break;
		}
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
	// nafなど計算できない値になったら
	if (!std::isfinite(worldTransform_.rotate.x) ||
		!std::isfinite(worldTransform_.rotate.y) ||
		!std::isfinite(worldTransform_.rotate.z) ||
		!std::isfinite(worldTransform_.rotate.w)) {
		worldTransform_.rotate = Quaternion::identity;
	}
}
