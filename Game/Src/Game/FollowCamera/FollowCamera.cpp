#include "FollowCamera.h"

#include "Engine/Input/Input.h"
#include "Engine/Math/MyMath.h"

void FollowCamera::Initialize() {}

void FollowCamera::Update() {
	if (target_) {
		interTarget_ = Lerp(interTarget_, MakeTranslateMatrix(target_->matWorld), 0.2f);
		GamePad();
		Keyboard();
		RotateUpdate();
		viewProjection_->UpdateMatrix();
	}
}

void FollowCamera::GamePad() {
	XINPUT_STATE joyState{};
	if (Input::GetInstance()->IsControllerConnected()) {
		Input::GetInstance()->GetJoystickState(0, joyState);
		// カメラの角度から回転行列を計算する
		// X軸
		viewProjection_->rotation_.x -=
			static_cast<float>(joyState.Gamepad.sThumbRY) * kRotateSpeedX;
		viewProjection_->rotation_.x = Clamp(
			viewProjection_->rotation_.x, DegToRad(kDeadZoneRotateMin),
			DegToRad(kDeadZoneRotateMax));
		// Y軸
		viewProjection_->rotation_.y +=
			static_cast<float>(joyState.Gamepad.sThumbRX) * kRotateSpeedY;

		destinationAngle_.y += static_cast<float>(joyState.Gamepad.sThumbRX) * kRotateSpeedY;
	}
}

void FollowCamera::Keyboard() {
	if (Input::GetInstance()->PushKey(DIK_RIGHTARROW)) {
		// Y軸を軸に回転
		viewProjection_->rotation_.y += kRotateSpeedY * kKeyboardCameraSpeed_X;
		destinationAngle_.y += kRotateSpeedY * kKeyboardCameraSpeed_X;
	}
	if (Input::GetInstance()->PushKey(DIK_LEFTARROW)) {
		// Y軸を軸に回転
		viewProjection_->rotation_.y -= kRotateSpeedY * kKeyboardCameraSpeed_X;
		destinationAngle_.y -= kRotateSpeedY * kKeyboardCameraSpeed_X;
	}

	if (Input::GetInstance()->PushKey(DIK_UPARROW)) {
		// X軸を軸に回転
		viewProjection_->rotation_.x += kRotateSpeedX * kKeyboardCameraSpeed_Y;
		viewProjection_->rotation_.x = Clamp(
			viewProjection_->rotation_.x, DegToRad(kDeadZoneRotateMin),
			DegToRad(kDeadZoneRotateMax));
		destinationAngle_.x += kRotateSpeedY * kKeyboardCameraSpeed_X;
	}
	if (Input::GetInstance()->PushKey(DIK_DOWNARROW)) {
		// X軸を軸に回転
		viewProjection_->rotation_.x -= kRotateSpeedX * kKeyboardCameraSpeed_Y;
		viewProjection_->rotation_.x = Clamp(
			viewProjection_->rotation_.x, DegToRad(kDeadZoneRotateMin),
			DegToRad(kDeadZoneRotateMax));
		destinationAngle_.x -= kRotateSpeedY * kKeyboardCameraSpeed_X;
	}
}

void FollowCamera::RotateUpdate() {
	// 最短角度補間
	viewProjection_->rotation_.y =
		LenpShortAngle(viewProjection_->rotation_.y, destinationAngle_.y, 0.2f);
	// 回転行列生成
	Matrix4x4 rotate =
		Mul(MakeRotateXMatrix(viewProjection_->rotation_.x),
			MakeRotateYMatrix(viewProjection_->rotation_.y));
	// オフセットをカメラの回転に合わせて回転させる
	Vector3 offset = TransformNormal({ 0.0f, 2.0f, -15.0f }, rotate);
	// 座標をコピーしてずらす/*interTarget_, target_->translation_*/
	viewProjection_->translation_ = interTarget_ + offset;
}
