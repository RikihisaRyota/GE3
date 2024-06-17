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
	ModelManager::GetInstance()->Load("Resources/Models/Bullet/bullet.gltf");
	playerModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Player/player.gltf");
	animation_.Initialize("Resources/Animation/Player/animation.gltf", playerModelHandle_);
	walkHandle_ = animation_.GetAnimationHandle("walk");
	worldTransform_.Initialize();
	animationTransform_.Initialize();

	playerUI_ = std::make_unique<PlayerUI>();

	JSON_OPEN("Resources/Data/Player/player.json");
	JSON_OBJECT("playerProperties");
	JSON_LOAD(reticleDistance_);
	JSON_LOAD(bulletLifeTime_);
	JSON_LOAD(bulletCoolTime_);
	JSON_ROOT();
	JSON_CLOSE();

#pragma region コライダー
	collider_ = new OBBCollider();
	collider_->SetName("Player");
	auto& mesh = ModelManager::GetInstance()->GetModel(playerModelHandle_).GetMeshData().at(0);
	Vector3 modelSize = mesh->meshes->max - mesh->meshes->min;
	collider_->SetCenter(MakeTranslateMatrix(worldTransform_.matWorld) + Vector3(0.0f, modelSize.y * 0.5f, 0.0f));
	collider_->SetOrientation(worldTransform_.rotate);
	collider_->SetSize({ modelSize.x * worldTransform_.scale.x,modelSize.y * worldTransform_.scale.y,modelSize.z * worldTransform_.scale.z });
	collider_->SetCallback([this](const ColliderDesc& collisionInfo) { OnCollision(collisionInfo); });
	collider_->SetCollisionAttribute(CollisionAttribute::Player);
	collider_->SetCollisionMask(~CollisionAttribute::Player);
	collider_->SetIsActive(true);
#pragma endregion
}

void Player::Initialize() {
	playerUI_->Initialize();
	worldTransform_.Reset();
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	UpdateTransform();
}

void Player::Update(CommandContext& commandContext) {
	colliderColor_ = { 0.0f,0.0f,1.0f,1.0f };
	Move();

	AnimationUpdate(commandContext);

	BulletUpdate();

	UpdateTransform();

	GPUParticleSpawn();

	playerUI_->Update();
}

void Player::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(animationTransform_, animation_, *viewProjection_, playerModelHandle_, commandContext);
	// 弾
	for (auto& bullet : playerBullets_) {
		bullet->Draw(viewProjection, commandContext);
	}
	playerUI_->Draw(commandContext);
}

void Player::DrawDebug(const ViewProjection& viewProjection) {
	// 弾
	for (auto& bullet : playerBullets_) {
		bullet->DrawDebug(viewProjection);
	}
	animation_.DrawLine(animationTransform_);
	collider_->DrawCollision(viewProjection, colliderColor_);
}

void Player::BulletUpdate() {
	Shot();
	// 弾の更新と生存状態の確認
	auto iter = playerBullets_.begin();
	while (iter != playerBullets_.end()) {
		// 弾を更新する
		(*iter)->Update();

		// 弾が生存していない場合は、リストから削除する
		if (!(*iter)->GetIsAlive()) {
			iter = playerBullets_.erase(iter); // 削除して、次の要素を指す
		}
		else {
			++iter; // 次の弾へ移動
		}
	}
}

void Player::Shot() {
	bulletTime_++;

	// レティクルのスクリーン座標
	Vector2 reticlePos = { 1280 * 0.5f, 720.0f * 0.5f };
	if (Input::GetInstance()->PushKey(DIK_SPACE) && bulletTime_ >= bulletCoolTime_) {
		bulletTime_ = 0;

		// プレイヤーの位置
		Vector3 playerPosition = MakeTranslateMatrix(worldTransform_.matWorld) * Vector3(1.0f, 5.0f, 1.0f);

		// VPV合成行列
		Matrix4x4 matVPV = viewProjection_->matView_ * viewProjection_->matProjection_ * MakeViewportMatrix(0.0f, 0.0f, float(WinApp::kWindowWidth), float(WinApp::kWindowHeight), viewProjection_->nearZ_, viewProjection_->farZ_);
		// 逆行列に
		Matrix4x4 inverseVPV = Inverse(matVPV);

		// スクリーン座標
		Vector3 posNear = Vector3(reticlePos.x, reticlePos.y, 0.0f);
		Vector3 posFar = Vector3(reticlePos.x, reticlePos.y, 1.0f);

		// スクリーンからワールド座標へ
		posNear = Transform(posNear, inverseVPV);
		posFar = Transform(posFar, inverseVPV);

		// レイの方向ベクトルを計算
		Vector3 rayDirection = Normalize(posFar - posNear);

		// 3Dレティクルの位置
		Vector3 reticle3D = posNear + (rayDirection * reticleDistance_);

		// 弾の速度ベクトルを計算
		Vector3 bulletVelocity = Normalize(reticle3D - playerPosition) * 0.5f;

		// 弾を作成
		playerBullets_.emplace_back(std::make_unique<PlayerBullet>());
		playerBullets_.back()->Create(gpuParticleManager_, playerPosition, bulletVelocity, bulletLifeTime_);
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
		// 回転行列生成
		Matrix4x4 rotate = MakeRotateYMatrix(viewProjection_->rotation_.y);
		// オフセットをカメラの回転に合わせて回転させる
		vector = TransformNormal(vector.Normalized(), rotate);
		worldTransform_.translate += vector * 0.1f;
		PlayerRotate(vector.Normalized());

		animationTime_ += 1.0f;
	}
}

void Player::DrawImGui() {
	ImGui::Begin("InGame");
	if (ImGui::BeginMenu("Player")) {
		ImGui::DragFloat3("position", &worldTransform_.translate.x, 0.1f);
		auto& material = ModelManager::GetInstance()->GetModel(playerModelHandle_).GetMaterialData();
		ImGui::DragFloat4("color", &material.color.x, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("environmentCoefficient", &material.environmentCoefficient, 0.1f, 0.0f, 1.0f);
		if (ImGui::TreeNode("PlayerProperties")) {
			ImGui::DragFloat("reticleDistance", &reticleDistance_, 0.1f, 0.0f);
			int time = bulletLifeTime_;
			ImGui::DragInt("bulletLifeTime_", &time, 1, 0);
			bulletLifeTime_ = time;
			time = bulletCoolTime_;
			ImGui::DragInt("bulletCoolTime_", &time, 1, 0);
			bulletCoolTime_ = time;
			if (ImGui::Button("Save")) {
				JSON_OPEN("Resources/Data/Player/player.json");
				JSON_OBJECT("playerProperties");
				JSON_SAVE(reticleDistance_);
				JSON_SAVE(bulletLifeTime_);
				JSON_SAVE(bulletCoolTime_);
				JSON_ROOT();
				JSON_CLOSE();
			}
			ImGui::TreePop();
		}
		ImGui::EndMenu();
	}
	ImGui::End();
}

void Player::AnimationUpdate(CommandContext& commandContext) {
	//static float cycle = 60.0f;
	//static float time = 0.0f;
	//// 1フレームでのパラメータ加算値
	//const float kFroatStep = 2.0f * std::numbers::pi_v<float> / cycle;
	//// パラメータを1ステップ分加算
	//time += kFroatStep;
	//// 2πを超えたら0に戻す
	//time = std::fmod(time, 2.0f * std::numbers::pi_v<float>);
	//// 浮遊を座標に反映
	//animationTransform_.translate.y = (std::sin(time) * 0.05f);
	static const float kCycle = 30.0f;
	animationTime_ = std::fmodf(animationTime_, kCycle);
	animation_.Update(walkHandle_, animationTime_ / kCycle, commandContext, playerModelHandle_);
}

void Player::PlayerRotate(const Vector3& move) {
	worldTransform_.rotate = Slerp(worldTransform_.rotate, MakeLookRotation(Vector3(move.x, 0.0f, move.z).Normalized()), 0.1f);
	/*Vector3 vector = Conjugation(worldTransform_.rotate) * move;
	if (Dot(Vector3(0.0f,0.0f,1.0f), vector) < 0.999f) {
		Quaternion diff = MakeRotateQuaternion(Vector3(0.0f,0.0f,1.0f), vector);
		worldTransform_.rotate= Slerp( Quaternion::identity, diff,0.1f) * worldTransform_.rotate;
	}*/
}
