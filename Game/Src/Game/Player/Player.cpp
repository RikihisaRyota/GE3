#include "Player.h"

#include <numbers>

#include "imgui.h"

#include "Engine/Model/ModelManager.h"
#include "Engine/Input/Input.h"
#include "Engine/Math/MyMath.h"
#include "Engine/Collision/CollisionAttribute.h"

Player::Player() {
	playerModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Walk/walk.gltf");
	animation_.Initialize(playerModelHandle_);

	worldTransform_.Initialize();
	animationTransform_.Initialize();
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
	worldTransform_.Reset();
	animationTransform_.Reset();
	animationTransform_.parent_ = &worldTransform_;
	UpdateTransform();
}

void Player::Update() {
	colliderColor_ = { 0.0f,0.0f,1.0f,1.0f };
	Move();

	AnimationUpdate();

	BulletUpdate();

	UpdateTransform();
#ifdef _DEBUG
	ImGui::Begin("InGame");
	if (ImGui::TreeNode("Player")) {
		ImGui::DragFloat3("position", &worldTransform_.translate.x, 0.1f);
		auto& material = ModelManager::GetInstance()->GetModel(playerModelHandle_).GetMaterialData();
		ImGui::DragFloat4("color", &material.color.x, 0.1f,0.0f,1.0f);
		ImGui::DragFloat("environmentCoefficient", &material.environmentCoefficient, 0.1f,0.0f,1.0f);
		
		ImGui::TreePop();
	}
	ImGui::End();
#endif // _DEBUG

}

void Player::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	ModelManager::GetInstance()->Draw(animationTransform_, animation_, *viewProjection_, playerModelHandle_, commandContext);
	//animation_.DrawBox(animationTransform_,viewProjection);
	animation_.DrawLine(animationTransform_);
	collider_->DrawCollision(viewProjection, colliderColor_);
	// 弾
	for (auto& bullet : playerBullets_) {
		bullet->Draw(viewProjection, commandContext);
	}
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
	if (Input::GetInstance()->PushKey(DIK_SPACE) && bulletTime_ >= kBulletCoolTime) {
		bulletTime_ = 0;
		playerBullets_.emplace_back(std::make_unique<PlayerBullet>());
		playerBullets_.back()->Create(gpuParticleManager_, MakeTranslateMatrix(worldTransform_.matWorld) * Vector3(1.0f, 5.0f, 1.0f), Normalize(GetZAxis(MakeRotate(worldTransform_.rotate))) * 0.5f, kBulletTime);
	}

}

void Player::OnCollision(const ColliderDesc& desc) {
	if (desc.collider->GetName() == "Boss") {
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
		vector.Normalized();
		// 回転行列生成
		Matrix4x4 rotate = MakeRotateYMatrix(viewProjection_->rotation_.y);
		// オフセットをカメラの回転に合わせて回転させる
		vector = TransformNormal(vector, rotate);
		worldTransform_.translate += vector * 0.2f;
		PlayerRotate(vector.Normalized());

		animationTime_ += 1.0f;
	}
}

void Player::AnimationUpdate() {
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
	animation_.Update(animationTime_ / kCycle);
}

void Player::PlayerRotate(const Vector3& move) {
	worldTransform_.rotate = Slerp(worldTransform_.rotate, MakeLookRotation({ move.x,0.0f,move.z }), 0.1f);
	/*Vector3 vector = Conjugation(worldTransform_.rotate) * move;
	if (Dot(Vector3(0.0f,0.0f,1.0f), vector) < 0.999f) {
		Quaternion diff = MakeRotateQuaternion(Vector3(0.0f,0.0f,1.0f), vector);
		worldTransform_.rotate= Slerp( Quaternion::identity, diff,0.1f) * worldTransform_.rotate;
	}*/
}
