#include "Player.h"

#include <numbers>

#include "imgui.h"

#include "Engine/Model/ModelManager.h"
#include "Engine/Input/Input.h"
#include "Engine/Math/MyMath.h"

Player::Player() {
	playerModelHandle_ = ModelManager::GetInstance()->Load("Resources/Models/Walk/walk.gltf");
	animation_.Initialize(playerModelHandle_);
}

void Player::Initialize() {
	worldTransform_.Initialize();
	animationTransform_.Initialize();
	animationTransform_.parent_ = &worldTransform_;
}

void Player::Update() {
	Move();

	AnimationUpdate();

	BulletUpdate();

	worldTransform_.UpdateMatrix();
	animationTransform_.UpdateMatrix();
#ifdef _DEBUG
	/*ImGui::Begin("Player");
	ImGui::DragFloat3("position",&worldTransform_.translation_.x,0.1f);
	ImGui::End();*/
#endif // _DEBUG

}

void Player::Draw(const ViewProjection& viewProjection, CommandContext& commandContext) {
	//ModelManager::GetInstance()->Draw(animationTransform_, animation_, *viewProjection_, playerModelHandle_, commandContext);
	//animation_.Draw(animationTransform_);
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
		vector.Normalize();
		// 回転行列生成
		Matrix4x4 rotate = MakeRotateYMatrix(viewProjection_->rotation_.y);
		// オフセットをカメラの回転に合わせて回転させる
		vector = TransformNormal(vector, rotate);
		worldTransform_.translate += vector * 0.2f;
		PlayerRotate(vector);
		
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

void Player::PlayerRotate(const Vector3& vector) {
	worldTransform_.rotate = Slerp( worldTransform_.rotate, MakeLookRotation({ vector.x,0.0f,vector.z }),0.1f);
}
